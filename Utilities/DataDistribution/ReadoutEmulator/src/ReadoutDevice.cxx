// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "ReadoutEmulator/ReadoutDevice.h"
#include "Common/ReadoutDataModel.h"

#include <options/FairMQProgOptions.h>
#include <FairMQLogger.h>

#include <chrono>
#include <thread>

namespace o2 {
namespace DataDistribution {

constexpr int gHbfOutputChanId = 0;

ReadoutDevice::ReadoutDevice()
  : O2Device{},
    mCruMemoryHandler{ std::make_shared<CruMemoryHandler>() },
    mRootApp("ReadoutApp", nullptr, nullptr),
    mReadoutCanvas("cnv", "Readout", 500, 400),
    mFreeSuperpagesSamples(10000)
{
}

ReadoutDevice::~ReadoutDevice()
{
}

void ReadoutDevice::InitTask()
{
  mOutChannelName = GetConfig()->GetValue<std::string>(OptionKeyOutputChannelName);
  mDataRegionSize = GetConfig()->GetValue<std::size_t>(OptionKeyReadoutDataRegionSize);

  mCruId = GetConfig()->GetValue<std::size_t>(OptionKeyCruId);

  mSuperpageSize = GetConfig()->GetValue<std::size_t>(OptionKeyCruSuperpageSize);
  mDmaChunkSize = GetConfig()->GetValue<std::size_t>(OptionKeyCruDmaChunkSize);

  mCruLinkCount = GetConfig()->GetValue<std::size_t>(OptionKeyCruLinkCount);
  mCruLinkBitsPerS = GetConfig()->GetValue<std::size_t>(OptionKeyCruLinkBitsPerS);

  ChannelAllocator::get().addChannel(gHbfOutputChanId, GetChannel(mOutChannelName, 0));

  if (mSuperpageSize < (1ULL << 19)) {
    LOG(WARN) << "Superpage size too low (" << mSuperpageSize << " Setting to 512kiB...";
    mSuperpageSize = (1ULL << 19);
  }

  if (mDmaChunkSize == 0) {
    mDmaChunkSize = (mCruLinkBitsPerS / 11223ULL) >> 3;
    LOG(INFO) << "Using HBFrame size of " << mDmaChunkSize << " and ignoring DMA Chunk size";
  }

  mDataRegion.reset();

  // Open SHM regions (segments)
  mDataRegion = NewUnmanagedRegionFor(
    mOutChannelName, 0, mDataRegionSize,
    [this](void* data, size_t size, void *hint) { // callback to be called when message buffers no longer needed by transport
      mCruMemoryHandler->put_data_buffer(static_cast<char*>(data), size);
    });

  LOG(INFO) << "Memory regions created";

  mCruMemoryHandler->init(mDataRegion.get(), mSuperpageSize, mDmaChunkSize);

  mCruLinks.clear();
  for (auto e = 0; e < mCruLinkCount; e++)
    mCruLinks.push_back(std::make_unique<CruLinkEmulator>(mCruMemoryHandler, mCruLinkBitsPerS, mDmaChunkSize));
}

void ReadoutDevice::PreRun()
{
  // start all cru link emulators
  for (auto& e : mCruLinks)
    e->start();

  // gui thread
  mGuiThread = std::thread(&ReadoutDevice::GuiThread, this);
}

void ReadoutDevice::PostRun()
{
  // stop all cru link emulators
  for (auto& e : mCruLinks)
    e->stop();
  // unblock waiters
  mCruMemoryHandler->teardown();
  mGuiThread.join();
}

bool ReadoutDevice::ConditionalRun()
{
  // finish an STF every ~1/45 seconds
  static const auto cDataTakingStart = std::chrono::high_resolution_clock::now();
  static const auto cStfInterval = std::chrono::microseconds(22222);
  static uint64_t lNumberSentStfs = 0;

  auto isStfFinished =
    (std::chrono::high_resolution_clock::now() - cDataTakingStart) - (lNumberSentStfs * cStfInterval) > cStfInterval;

  if (isStfFinished)
    lNumberSentStfs += 1;

  ReadoutLinkO2Data lCruLinkData;
  if (!mCruMemoryHandler->getLinkData(lCruLinkData)) {
    LOG(INFO) << "GetLinkData failed. Stopping interface thread.";
    return false;
  }

  if (mBuildHistograms)
    mFreeSuperpagesSamples.Fill(mCruMemoryHandler->free_superpages());

  // check no data signal
  if (lCruLinkData.mLinkDataHeader.subSpecification == -1) {
    // LOG(WARN) << "No Superpages left! Losing data...";
    return true;
  }

  ReadoutSubTimeframeHeader lHBFHeader;
  lHBFHeader.timeframeId = lNumberSentStfs;
  lHBFHeader.numberOfHBF = lCruLinkData.mLinkRawData.size();
  lHBFHeader.linkId = lCruLinkData.mLinkDataHeader.subSpecification;

  auto lHdrMsg = NewMessageFor(mOutChannelName, 0, sizeof(ReadoutSubTimeframeHeader));
  std::memcpy(lHdrMsg->GetData(), &lHBFHeader, sizeof(ReadoutSubTimeframeHeader));
  Send(lHdrMsg, mOutChannelName, 0);

  // create messages for the data
  for (const auto& lDmaChunk : lCruLinkData.mLinkRawData) {
    // mark this as used in the memory handler
    mCruMemoryHandler->get_data_buffer(lDmaChunk.mDataPtr, lDmaChunk.mDataSize);

    // create a message out of unmanaged region
    auto lDataMsg = NewMessageFor(mOutChannelName, 0, mDataRegion, lDmaChunk.mDataPtr, lDmaChunk.mDataSize);

    Send(lDataMsg, mOutChannelName, 0);
  }

  return true;
}

void ReadoutDevice::GuiThread()
{
  while (CheckCurrentState(RUNNING)) {
    auto lHistTitle = "[Readout-" + std::to_string(mCruId) + "] Number of free superpages";
    TH1F lFreeSuperpagesHist("SPCountH", lHistTitle.c_str(), 64, 0.0, mDataRegionSize / mSuperpageSize);
    lFreeSuperpagesHist.GetXaxis()->SetTitle("Count");
    for (const auto v : mFreeSuperpagesSamples)
      lFreeSuperpagesHist.Fill(v);

    mReadoutCanvas.cd(1);
    lFreeSuperpagesHist.Draw();

    mReadoutCanvas.Modified();
    mReadoutCanvas.Update();

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(5s);
  }
  LOG(INFO) << "Exiting GUI thread...";
}

}
} /* namespace o2::DataDistribution */

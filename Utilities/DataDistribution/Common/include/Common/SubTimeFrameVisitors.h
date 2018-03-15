// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef ALICEO2_SUBTIMEFRAME_VISITORS_H_
#define ALICEO2_SUBTIMEFRAME_VISITORS_H_

#include "Common/SubTimeFrameDataModel.h"

#include <Headers/DataHeader.h>

class O2Device;

#include <deque>
#include <vector>

class FairMQChannel;

namespace o2 {
namespace DataDistribution {

////////////////////////////////////////////////////////////////////////////////
/// InterleavedHdrDataSerializer
////////////////////////////////////////////////////////////////////////////////

class InterleavedHdrDataSerializer : public ISubTimeFrameVisitor {
public:
  InterleavedHdrDataSerializer() = delete;
  InterleavedHdrDataSerializer(const FairMQChannel& pChan) : mChan(pChan)
  {
    mMessages.reserve(1024);
  }

  void serialize(SubTimeFrame&& pStf);

protected:
  void visit(EquipmentHBFrames& pStf) override;
  void visit(SubTimeFrame& pStf) override;

private:
  std::vector<FairMQMessagePtr> mMessages;
  const FairMQChannel& mChan;
};

////////////////////////////////////////////////////////////////////////////////
/// InterleavedHdrDataDeserializer
////////////////////////////////////////////////////////////////////////////////

class InterleavedHdrDataDeserializer : public ISubTimeFrameVisitor {
public:
  InterleavedHdrDataDeserializer() = delete;
  InterleavedHdrDataDeserializer(const FairMQChannel& pChan) : mChan(pChan)
  {
  }

  bool deserialize(SubTimeFrame& pStf);

protected:
  void visit(EquipmentHBFrames& pStf) override;
  void visit(SubTimeFrame& pStf) override;

private:
  std::deque<FairMQMessagePtr> mMessages;
  const FairMQChannel& mChan;
};

////////////////////////////////////////////////////////////////////////////////
/// HdrDataSerializer
////////////////////////////////////////////////////////////////////////////////

class HdrDataSerializer : public ISubTimeFrameVisitor {
public:
  HdrDataSerializer() = delete;
  HdrDataSerializer(const FairMQChannel& pChan) : mChan(pChan)
  {
    mHeaderMessages.reserve(1024);
    mDataMessages.reserve(1024);
  }

  void serialize(SubTimeFrame &&pStf);

protected:
  void visit(EquipmentHBFrames& pStf) override;
  void visit(SubTimeFrame& pStf) override;

private:
  std::vector<FairMQMessagePtr> mHeaderMessages;
  std::vector<FairMQMessagePtr> mDataMessages;
  const FairMQChannel& mChan;
};

////////////////////////////////////////////////////////////////////////////////
/// HdrDataVisitor
////////////////////////////////////////////////////////////////////////////////

class HdrDataDeserializer : public ISubTimeFrameVisitor {
public:
  HdrDataDeserializer() = delete;
  HdrDataDeserializer(const FairMQChannel& pChan) : mChan(pChan)
  {
  }

  bool deserialize(SubTimeFrame& pStf);

protected:
  void visit(EquipmentHBFrames& pStf) override;
  void visit(SubTimeFrame& pStf) override;

private:
  std::deque<FairMQMessagePtr> mHeaderMessages;
  std::deque<FairMQMessagePtr> mDataMessages;
  const FairMQChannel& mChan;
};


}
} /* o2::DataDistribution */

#endif /* ALICEO2_SUBTIMEFRAME_VISITORS_H_ */

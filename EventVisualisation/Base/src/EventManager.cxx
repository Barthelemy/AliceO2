// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See https://alice-o2.web.cern.ch/ for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// \file    EveInitializer.cxx
/// \brief   Implementation of the EventManager class.
/// \author  Jeremi Niedziela

#include "EventManager.h"

#include "MultiView.h"

#include <TEveManager.h>
#include <TEveProjectionManager.h>
#include <TSystem.h>

using namespace std;

namespace o2  {
namespace Eve {

EventManager* EventManager::sMaster  = nullptr;

EventManager* EventManager::Instance()
{
  if(!sMaster){
    new EventManager();
  }
  return sMaster;
}

void EventManager::RegisterEvent(TEveElement* event)
{
  auto multiView = MultiView::Instance();
  
  gEve->AddElement(event,multiView->GetScene(MultiView::Scene3dEvent));
  multiView->GetProjection(MultiView::ProjectionRphi)->ImportElements(event,multiView->GetScene(MultiView::SceneRphiEvent));
  multiView->GetProjection(MultiView::ProjectionZrho)->ImportElements(event,multiView->GetScene(MultiView::SceneZrhoEvent));
}

void EventManager::DestroyAllEvents()
{
  auto multiView = MultiView::Instance();
  
  multiView->GetScene(MultiView::Scene3dEvent)->DestroyElements();
  multiView->GetScene(MultiView::SceneRphiEvent)->DestroyElements();
  multiView->GetScene(MultiView::SceneZrhoEvent)->DestroyElements();
}

EventManager::EventManager() : TEveEventManager("Event",""),
mCurrentDataSourceType(SourceOffline)
{
  sMaster = this;
}

EventManager::~EventManager()
{
}
  
}
}

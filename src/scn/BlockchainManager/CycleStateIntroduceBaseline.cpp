/*
 * This file is part of SwabianCoin.
 *
 * SwabianCoin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * SwabianCoin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SwabianCoin.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "CycleStateIntroduceBaseline.h"
#include "BlockchainManager.h"
#include "scn/Blockchain/Blockchain.h"

using namespace scn;

CycleStateIntroduceBaseline::CycleStateIntroduceBaseline(BlockchainManager& base)
:base_(base) {

}


CycleStateIntroduceBaseline::~CycleStateIntroduceBaseline() = default;


void CycleStateIntroduceBaseline::onEnter() {
    LOG(INFO) << "enter introduce baseline state";
    base_.pauseMiner();
    base_.blockchain_.establishBaseline();
    base_.resumeMiner();
}


bool CycleStateIntroduceBaseline::onCycle() {
    return false;
}


void CycleStateIntroduceBaseline::onExit() {

}
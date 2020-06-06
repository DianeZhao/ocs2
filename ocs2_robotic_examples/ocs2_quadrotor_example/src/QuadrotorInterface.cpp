/******************************************************************************
Copyright (c) 2020, Farbod Farshidian. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

 * Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

#include "ocs2_quadrotor_example/QuadrotorInterface.h"
#include <ros/package.h>

namespace ocs2 {
namespace quadrotor {

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
QuadrotorInterface::QuadrotorInterface(const std::string& taskFileFolderName)
    : Q_(STATE_DIM_, STATE_DIM_),
      R_(INPUT_DIM_, INPUT_DIM_),
      QFinal_(STATE_DIM_, STATE_DIM_),
      xFinal_(STATE_DIM_),
      xNominal_(STATE_DIM_),
      uNominal_(INPUT_DIM_),
      initialState_(STATE_DIM_) {
  taskFile_ = ros::package::getPath("ocs2_quadrotor_example") + "/config/" + taskFileFolderName + "/task.info";
  std::cerr << "Loading task file: " << taskFile_ << std::endl;

  libraryFolder_ = ros::package::getPath("ocs2_quadrotor_example") + "/auto_generated";
  std::cerr << "Generated library path: " << libraryFolder_ << std::endl;

  // load setting from loading file
  loadSettings(taskFile_);
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void QuadrotorInterface::loadSettings(const std::string& taskFile) {
  /*
   * Default initial condition
   */
  loadData::loadEigenMatrix(taskFile, "initialState", initialState_);

  /*
   * Solver settings
   */
  ilqrSettings_.loadSettings(taskFile);
  mpcSettings_.loadSettings(taskFile);

  /*
   * quadrotor parameters
   */
  QuadrotorParameters quadrotorParameters;
  quadrotorParameters.loadSettings(taskFile);

  /*
   * Dynamics and derivatives
   */
  quadrotorSystemDynamicsPtr_.reset(new QuadrotorSystemDynamics(quadrotorParameters));

  /*
   * Rollout
   */
  Rollout_Settings rolloutSettings;
  rolloutSettings.loadSettings(taskFile, "slq.rollout");
  ddpQuadrotorRolloutPtr_.reset(new TimeTriggeredRollout(STATE_DIM_, INPUT_DIM_, *quadrotorSystemDynamicsPtr_, rolloutSettings));

  /*
   * Cost function
   */
  loadData::loadEigenMatrix(taskFile, "Q", Q_);
  loadData::loadEigenMatrix(taskFile, "R", R_);
  loadData::loadEigenMatrix(taskFile, "Q_final", QFinal_);
  loadData::loadEigenMatrix(taskFile, "x_final", xFinal_);
  xNominal_ = vector_t::Zero(STATE_DIM_);
  uNominal_ = vector_t::Zero(INPUT_DIM_);

  std::cerr << "Q:  \n" << Q_ << std::endl;
  std::cerr << "R:  \n" << R_ << std::endl;
  std::cerr << "Q_final:\n" << QFinal_ << std::endl;
  std::cerr << "x_init:   " << initialState_.transpose() << std::endl;
  std::cerr << "x_final:  " << xFinal_.transpose() << std::endl;

  quadrotorCostPtr_.reset(new QuadrotorCost(Q_, R_, xNominal_, uNominal_, QFinal_, xFinal_));

  /*
   * Constraints
   */
  quadrotorConstraintPtr_.reset(new ConstraintBase(STATE_DIM_, INPUT_DIM_));

  /*
   * Initialization
   */
  vector_t initialInput = vector_t::Zero(INPUT_DIM_);
  initialInput(0) = quadrotorParameters.quadrotorMass_ * quadrotorParameters.gravity_;
  quadrotorOperatingPointPtr_.reset(new OperatingPoints(initialState_, initialInput));

  /*
   * Time partitioning which defines the time horizon and the number of data partitioning
   */
  scalar_t timeHorizon;
  ocs2::loadData::loadPartitioningTimes(taskFile, timeHorizon, numPartitions_, partitioningTimes_, true);
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
std::unique_ptr<MPC_ILQR> QuadrotorInterface::getMpc() {
  return std::unique_ptr<MPC_ILQR>(new MPC_ILQR(STATE_DIM_, INPUT_DIM_, ddpQuadrotorRolloutPtr_.get(), quadrotorSystemDynamicsPtr_.get(),
                                                quadrotorConstraintPtr_.get(), quadrotorCostPtr_.get(), quadrotorOperatingPointPtr_.get(),
                                                partitioningTimes_, ilqrSettings_, mpcSettings_));
}

}  // namespace quadrotor
}  // namespace ocs2

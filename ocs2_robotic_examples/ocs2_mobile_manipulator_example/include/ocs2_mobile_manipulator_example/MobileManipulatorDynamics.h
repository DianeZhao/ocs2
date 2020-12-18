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

#pragma once

#include <ocs2_core/dynamics/SystemDynamicsBaseAD.h>

#include <ocs2_mobile_manipulator_example/definitions.h>
#include <ocs2_pinocchio/PinocchioInterface.h>

namespace mobile_manipulator {

class MobileManipulatorDynamics final : public ocs2::SystemDynamicsBaseAD {
 public:
  using ad_scalar_t = ocs2::ad_scalar_t;
  using ad_vector_t = ocs2::ad_vector_t;

  explicit MobileManipulatorDynamics(const ocs2::PinocchioInterfaceCppAd& pinocchioInterface);
  ~MobileManipulatorDynamics() override = default;

  /* Copy constructor */
  MobileManipulatorDynamics(const MobileManipulatorDynamics& rhs) : ocs2::SystemDynamicsBaseAD(rhs) {
    pinocchioInterface_.reset(new ocs2::PinocchioInterfaceCppAd(*rhs.pinocchioInterface_));
  }

  /* Clone */
  MobileManipulatorDynamics* clone() const override { return new MobileManipulatorDynamics(*this); }

  ad_vector_t systemFlowMap(ad_scalar_t time, const ad_vector_t& state, const ad_vector_t& input) const override;

 private:
  std::unique_ptr<ocs2::PinocchioInterfaceCppAd> pinocchioInterface_;
};

}  // namespace mobile_manipulator

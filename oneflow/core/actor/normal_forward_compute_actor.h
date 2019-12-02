#ifndef ONEFLOW_CORE_ACTOR_NORMAL_FORWARD_COMPUTE_ACTOR_H_
#define ONEFLOW_CORE_ACTOR_NORMAL_FORWARD_COMPUTE_ACTOR_H_

#include "oneflow/core/actor/compute_actor.h"

namespace oneflow {

class NormalForwardCompActor final : public CompActor {
 public:
  OF_DISALLOW_COPY_AND_MOVE(NormalForwardCompActor);
  NormalForwardCompActor() = default;
  ~NormalForwardCompActor() override = default;

 private:
  void VirtualCompActorInit(const TaskProto&) override;
  void ForEachCurCustomizedReadableRegst(std::function<void(const Regst*)>) const override;
  void Act() override;
  std::pair<RegstNameType, HashSet<std::string>> GetNaiveOrCustomizedConsumedRegstDescName()
      override {
    return std::make_pair(RegstNameType::kNaive, HashSet<std::string>{"in"});
  }
  std::pair<RegstNameType, HashSet<std::string>> GetNaiveOrCustomizedProducedRegstDescName()
      override {
    return std::make_pair(RegstNameType::kCustomized, HashSet<std::string>{"const_buf"});
  }
  void AsyncSendCustomizedProducedRegstMsgToConsumer() override {}
  void VirtualAsyncSendNaiveProducedRegstMsgToConsumer() override;
  void VirtualAsyncSendInplaceProducedRegstMsgToConsumer() override;
  void AsyncSendCustomizedConsumedRegstMsgToProducer() override;

  bool IsCustomizedWriteReady() const override;
  void UpdtStateAsCustomizedProducedRegst(Regst* regst) override;
  bool CheckOutputActId(int64_t regst_desc_id) const override;

  void SendConstBufInitMsgToBwActor();

  int64_t cur_piece_id_;

  // customized produced
  int64_t const_buf_regst_desc_id_;
  Regst* const_buf_regst_;
  bool send_const_buf_regst_;
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_ACTOR_NORMAL_FORWARD_COMPUTE_ACTOR_H_

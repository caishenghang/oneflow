/*
Copyright 2020 The OneFlow Authors. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include "oneflow/core/framework/framework.h"
#include "oneflow/core/common/balanced_splitter.h"
#include "oneflow/user/ops/nn_util.h"

namespace oneflow {

namespace{


Maybe<void> GetOpSbpSignature(user_op::SbpContext* ctx) {
    const user_op::TensorDesc& x_tensor = ctx->LogicalTensorDesc4InputArgNameAndIndex("x", 0);
      const auto& padding = ctx->Attr<std::vector<int64_t>>("padding");
      FOR_RANGE(int64_t, i, 0, x_tensor.shape().NumAxes()) {
        ctx->NewBuilder()
              .Split(user_op::OpArg("x", 0), i)
              .Split(user_op::OpArg("y", 0), i)
              .Build();
      }
      return Maybe<void>::Ok();
}


Maybe<void> GetOpGradSbpSignature(user_op::SbpContext* ctx) {
    const user_op::TensorDesc& dy_tensor = ctx->LogicalTensorDesc4InputArgNameAndIndex("dy", 0);
      const auto& padding = ctx->Attr<std::vector<int64_t>>("padding");
      FOR_RANGE(int64_t, i, 0, dy_tensor.shape().NumAxes()) {
        ctx->NewBuilder()
              .Split(user_op::OpArg("dx", 0), i)
              .Split(user_op::OpArg("dy", 0), i)
              .Build();
      }
      return Maybe<void>::Ok();
}

}


REGISTER_USER_OP("reflection_pad2d")
    .Input("x")
    .Output("y")
    .Attr<std::string>("data_format")
    .Attr<std::vector<int64_t>>("padding")
    .SetTensorDescInferFn([](user_op::InferContext * ctx) -> Maybe<void>{
        Shape* x_shape = ctx->Shape4ArgNameAndIndex("x", 0);
        const auto& padding = ctx->Attr<std::vector<int64_t>>("padding");
        const std::string  data_format = ctx->Attr<std::string>("data_format");
        CHECK_EQ_OR_RETURN(padding.size(), x_shape->NumAxes());
        int channel_h_idx, channel_w_idx;
        if(data_format=="NCHW"){
            channel_h_idx=2;
            channel_w_idx=3;
        }else{
            channel_h_idx=1;
            channel_w_idx=2;
        }
        CHECK_LT_OR_RETURN(padding[channel_h_idx], x_shape->At(channel_h_idx));
        CHECK_LT_OR_RETURN(padding[channel_w_idx], x_shape->At(channel_w_idx));

        DimVector y_dim_vec(x_shape->NumAxes());
        int64_t h_x, w_x;
        h_x = x_shape->At(channel_h_idx);
        w_x = x_shape->At(channel_w_idx);
        y_dim_vec[0] = x_shape->At(0);
        y_dim_vec[channel_h_idx] = h_x + 2*padding[channel_h_idx];
        y_dim_vec[channel_w_idx] = w_x +  2*padding[channel_w_idx];
        if (data_format=="NCHW"){
            y_dim_vec[1] = x_shape->At(1);
        }else{
            y_dim_vec[3] = x_shape->At(3);
        }
        *ctx->Shape4ArgNameAndIndex("y", 0) = Shape(y_dim_vec);
        *ctx->Dtype4ArgNameAndIndex("y", 0) = *ctx->Dtype4ArgNameAndIndex("x", 0);
        return Maybe<void>::Ok();
    })
    .SetGetSbpFn(GetOpSbpSignature)
    .SetInputArgModifyFn([](user_op::GetInputArgModifier GetInputArgModifierFn,
                            const user_op::UserOpConfWrapper&) {
        user_op::InputArgModifier* x_modifier = GetInputArgModifierFn("x", 0);
        CHECK_NOTNULL(x_modifier);
        x_modifier->set_requires_grad(false);
    });


REGISTER_USER_OP("reflection_pad2d_grad")
    .Input("dy")
    .Output("dx")
    .Attr<std::string>("data_format")
    .Attr<std::vector<int64_t>>("padding")
    .SetTensorDescInferFn([](user_op::InferContext * ctx) -> Maybe<void>{
        Shape* dy_shape = ctx->Shape4ArgNameAndIndex("dy", 0);
        const auto& padding = ctx->Attr<std::vector<int64_t>>("padding");
        const std::string  data_format = ctx->Attr<std::string>("data_format");
        CHECK_EQ_OR_RETURN(padding.size(), dy_shape->NumAxes());
        int channel_h_idx, channel_w_idx;
        if(data_format=="NCHW"){
            channel_h_idx=2;
            channel_w_idx=3;
        }else{
            channel_h_idx=1;
            channel_w_idx=2;
        }
        CHECK_LT_OR_RETURN(padding[channel_h_idx], dy_shape->At(channel_h_idx));
        CHECK_LT_OR_RETURN(padding[channel_w_idx], dy_shape->At(channel_w_idx));

        DimVector dx_dim_vec(dy_shape->NumAxes());
        int64_t h_dy, w_dy;
        h_dy = dy_shape->At(channel_h_idx);
        w_dy = dy_shape->At(channel_w_idx);
        dx_dim_vec[0] = dy_shape->At(0);
        dx_dim_vec[channel_h_idx] = h_dy - 2*padding[channel_h_idx];
        dx_dim_vec[channel_w_idx] = w_dy -  2*padding[channel_w_idx];
        if (data_format=="NCHW"){
            dx_dim_vec[1] = dy_shape->At(1);
        }else{
            dx_dim_vec[3] = dy_shape->At(3);
        }
        *ctx->Shape4ArgNameAndIndex("dx", 0) = Shape(dx_dim_vec);
        *ctx->Dtype4ArgNameAndIndex("dx", 0) = *ctx->Dtype4ArgNameAndIndex("dy", 0);
        return Maybe<void>::Ok();
    })
    .SetGetSbpFn(GetOpGradSbpSignature);


REGISTER_USER_OP_GRAD("reflection_pad2d")
    .SetGenBackwardOpConfFn([](const user_op::UserOpWrapper& op,
                                                    user_op::AddOpFn AddOp) {
    if (op.NeedGenGradTensor4OpInput("x", 0)) {
        user_op::UserOpConfWrapperBuilder builder(op.op_name() + "_grad");
        user_op::UserOpConfWrapper grad_op =
            builder.Op("reflection_pad2d_grad")
                .Input("dy", op.GetGradTensorWithOpOutput("y", 0))
                .Output("dx")
                .Attr("data_format", op.attr<std::string>("data_format"))
                .Attr("padding", op.attr<std::vector<int64_t>>("padding"))
                .Build();
        op.BindGradTensorWithOpInput(grad_op.output("dx", 0), "x", 0);
        AddOp(grad_op);
    }
});


REGISTER_USER_OP_GRAD("reflection_pad2d_grad")
    .SetGenBackwardOpConfFn([](const user_op::UserOpWrapper& op, user_op::AddOpFn AddOp) {
    if (op.NeedGenGradTensor4OpInput("dy", 0)) {
        user_op::UserOpConfWrapperBuilder builder(op.op_name() + "_grad");
        user_op::UserOpConfWrapper grad_op =
            builder.Op("reflection_pad2d")
                .Input("x", op.GetGradTensorWithOpOutput("dx", 0))
                .Output("y")
                .Attr("data_format", op.attr<std::string>("data_format"))
                .Attr("padding", op.attr<std::vector<int64_t>>("padding"))
                .Build();
        op.BindGradTensorWithOpInput(grad_op.output("y", 0), "dy", 0);
        AddOp(grad_op);
    }
});


}  // namespace oneflow
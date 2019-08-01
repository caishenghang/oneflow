#include "tensorflow/compiler/xla/client/xla_builder.h"
#include "tensorflow/compiler/xla/client/lib/math.h"
#include "tensorflow/compiler/xla/client/lib/constants.h"
#include "oneflow/xla/of2xla/xla_op_compiler_registry.h"
#include "oneflow/xla/of2xla/xla_op_compiler.h"
#include "oneflow/xla/of2xla/xla_op_context.h"

#include "oneflow/xla/of2xla/ops/unary_op.h"

namespace oneflow {
namespace mola {

template <typename UnaryOp>
class ApplyUnaryOp : public XlaOpCompiler {
 public:
  void Compile(XlaOpContext *ctx) override {
    ctx->SetOutput("out", UnaryOp()(ctx->Input("in")));
  }
};

REGISTER_XLA_OP_COMPILER(Sigmoid, ApplyUnaryOp<op::Logistic>);
REGISTER_XLA_OP_COMPILER(Tanh, ApplyUnaryOp<op::Tanh>);

struct Gelu {
  xla::XlaOp operator()(xla::XlaOp x) {
    xla::XlaOp dot_5 = xla::ScalarLike(x, 0.5);
    xla::XlaOp one = xla::ScalarLike(x, 1.0);
    // cdf = erf(sqrt(0.5) * x)
    xla::XlaOp cdf = xla::Erf(xla::Mul(xla::Sqrt(dot_5), x));
    // return 0.5 * x * (1.0 + cdf)
    return xla::Mul(xla::Mul(dot_5, x), xla::Add(one, cdf));
  }
};

REGISTER_XLA_OP_COMPILER(Gelu, ApplyUnaryOp<Gelu>);

}  // namespace mola
}  // namespace oneflow

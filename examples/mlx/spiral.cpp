// Train a two-layer network on a three-arm spiral with Apple's MLX, on the CPU, and print the loss curve and accuracy.
#include <cstdio>
#include <functional>
#include <vector>
#include "mlx/mlx.h"

namespace mx = mlx::core;

static float item(const mx::array& a) { mx::eval(a); return a.item<float>(); }

int main() {
    mx::set_default_device(mx::Device(mx::Device::cpu));
    const int per_class = 200, classes = 3, hidden = 64, steps = 400;
    const float lr = 0.5f;

    // The spiral: arm c sweeps radius 0..1 while its angle turns 4 radians, with Gaussian jitter on the angle.
    std::vector<mx::array> xs, ys;
    auto key = mx::random::key(20261102);
    for (int c = 0; c < classes; ++c) {
        auto parts = mx::random::split(key); key = parts.first;
        auto r = mx::linspace(0.0, 1.0, per_class);
        auto theta = r * 4.0f + float(c) * 4.18879f + mx::random::normal({per_class}, 0.0f, 0.2f, parts.second);
        xs.push_back(mx::stack({r * mx::sin(theta), r * mx::cos(theta)}, 1));
        ys.push_back(mx::full({per_class}, c, mx::int32));
    }
    auto X = mx::concatenate(xs, 0), Y = mx::concatenate(ys, 0);

    auto k1 = mx::random::split(key), k2 = mx::random::split(k1.first);
    std::vector<mx::array> params = {
        mx::random::normal({2, hidden}, 0.0f, 0.5f, k1.second), mx::zeros({hidden}),
        mx::random::normal({hidden, classes}, 0.0f, 0.5f, k2.second), mx::zeros({classes})};

    auto logits_of = [&](const std::vector<mx::array>& p) {
        return mx::matmul(mx::tanh(mx::matmul(X, p[0]) + p[1]), p[2]) + p[3];
    };
    auto loss_of = [&](const std::vector<mx::array>& p) {
        auto z = logits_of(p);
        auto picked = mx::take_along_axis(z, mx::expand_dims(Y, 1), 1);
        return mx::mean(mx::logsumexp(z, std::vector<int>{1}, true) - picked);
    };
    auto step = mx::value_and_grad(std::function<mx::array(const std::vector<mx::array>&)>(loss_of), std::vector<int>{0, 1, 2, 3});

    std::printf("MLX %s, device cpu, %d points, %d hidden units\n", mx::version(), per_class * classes, hidden);
    for (int s = 0; s <= steps; ++s) {
        auto [loss, grads] = step(params);
        if (s % 50 == 0) std::printf("step %3d  loss %.4f\n", s, item(loss));
        for (size_t i = 0; i < params.size(); ++i) params[i] = params[i] - lr * grads[i];
        mx::eval(params);
    }
    auto acc = mx::mean(mx::astype(mx::equal(mx::argmax(logits_of(params), 1), Y), mx::float32));
    std::printf("accuracy %.4f\n", item(acc));
    auto probe = mx::softmax(mx::matmul(mx::tanh(mx::matmul(mx::array({0.5f, -0.25f}, {1, 2}), params[0]) + params[1]), params[2]) + params[3], std::vector<int>{1});
    mx::eval(probe);
    std::printf("class probabilities at (0.5, -0.25): %.4f %.4f %.4f\n", probe.data<float>()[0], probe.data<float>()[1], probe.data<float>()[2]);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2015 Microsoft Corporation. All rights reserved.
// Copyright (C) 2021 blu3mania <blu3mania@hotmail.com>
//
// This code is licensed under the MIT License (MIT).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>

// This class is modified from Microsoft's gsl::util.
// The major change is to remove the use of template to reduce the number of generated
// template classes -- I can't think of a reasonable scenario where a finally block needs
// more than a single function call of void().
//

namespace utility {

  using final_action_t = std::function<void()>;

  // FinalAction allows you to ensure something gets run at the end of a scope.
  // "nodiscard" is applied since the returned FinalAction should not be discard immediately
  // as that will trigger the destructor and cause unexpected behavior.
  //
  class FinalAction {
    public:
      [[nodiscard]] inline explicit FinalAction(final_action_t func) noexcept : func(std::move(func)) {}
      inline FinalAction(FinalAction&& other) noexcept : func(std::move(other.func)), valid(std::exchange(other.valid, false)) {}

      FinalAction(const FinalAction&) = delete;
      FinalAction& operator=(const FinalAction&) = delete;
      FinalAction& operator=(FinalAction&&) = delete;

      inline ~FinalAction() noexcept {
        if (valid) {
          func();
        }
      }

  private:
      final_action_t func;
      bool valid {true};
  };

  // finally() - convenience function to generate a FinalAction
  [[nodiscard]] inline FinalAction finally(final_action_t&& func) noexcept { return FinalAction(std::forward<final_action_t>(func)); }

} // namespace
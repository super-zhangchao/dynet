#ifndef DYNET_C_INTERNAL_H_
#define DYNET_C_INTERNAL_H_

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// #include <dynet/device.h>
// #include <dynet/error.h>
// #include <dynet/graph.h>
// #include <dynet/initializer.h>
// #include <dynet/model.h>
// #include <dynet/parameter.h>
#include <dynet/dim.h>
// #include <dynet/tensor.h>
// #include <dynet/optimizer.h>

#include <dynet_c/define.h>

#define DYNET_C_PTR_TO_PTR(cpp_name, c_name) \
inline c_name *to_c_ptr(dynet::cpp_name *instance) { \
  return reinterpret_cast<c_name*>(instance); \
} \
inline const c_name *to_c_ptr(const dynet::cpp_name *instance) { \
  return reinterpret_cast<const c_name*>(instance); \
} \
inline dynet::cpp_name *to_cpp_ptr(c_name *instance) { \
  return reinterpret_cast<dynet::cpp_name*>(instance); \
} \
inline const dynet::cpp_name *to_cpp_ptr(const c_name *instance) { \
  return reinterpret_cast<const dynet::cpp_name*>(instance); \
}

#define DYNET_C_VAL_TO_PTR(cpp_name, c_name) \
inline c_name *to_c_ptr_from_value(dynet::cpp_name &&instance) { \
  return reinterpret_cast<c_name*>( \
      new dynet::cpp_name(std::forward<dynet::cpp_name>(instance))); \
}

#define DYNET_C_HANDLE_EXCEPTIONS \
catch (const std::exception &e) { \
  return dynet_c::internal::ErrorHandler::get_instance().handle(e); \
}

#define DYNET_C_CHECK_NOT_NULL(var) \
if (!var) { \
  DYNET_C_THROW_ERROR("Argument `" #var "` must not be null."); \
}

#define DYNET_C_THROW_ERROR(cmds) { \
  std::stringstream ss; \
  ss << cmds; \
  throw dynet_c::internal::Error(__FILE__, __LINE__, ss.str()); \
}

// struct dynetDevice;
// struct dynetNode;
// struct dynetGraph;
// struct dynetInitializer;
// struct dynetModel;
// struct dynetParameter;
struct dynetDim;
// struct dynetTensor;
// struct dynetOptimizer;

namespace dynet_c {

namespace internal {

class Error : public std::exception {
  Error() = delete;

public:
  Error(const std::string &file, std::uint32_t line, const std::string &message)
  : file_(file), line_(line), msg_(message) {
    std::stringstream ss;
    ss << file_ << ": " << line_ << ": " << msg_;
    full_msg_ = ss.str();
  }

  const char *what() const noexcept override { return full_msg_.c_str(); }

private:
  std::string file_;
  std::uint32_t line_;
  std::string msg_;
  std::string full_msg_;
};

template<typename T>
using Throwable = typename std::enable_if<
    std::is_base_of<std::exception, T>::value>::type;

class ErrorHandler {
 public:
  ErrorHandler() noexcept : exception_(nullptr), message_("OK") {}
  ~ErrorHandler() = default;

  template<typename T, typename = Throwable<T>>
  ::DYNET_C_STATUS handle(const T &e) {
    exception_ = std::make_exception_ptr(e);
    message_ = e.what();
    return DYNET_C_ERROR;
  }

  std::exception rethrow() {
    if (has_exception()) {
      std::rethrow_exception(exception_);
    } else {
      throw std::bad_exception();
    }
  }

  void reset() noexcept {
    exception_ = nullptr;
    message_ = "OK";
  }

  bool has_exception() const noexcept {
    return !exception_;
  }

  const char *get_message() const noexcept {
    return message_.c_str();
  }

  static ErrorHandler &get_instance();

 private:
  std::exception_ptr exception_;
  std::string message_;
};

// DYNET_C_PTR_TO_PTR(Device, dynetDevice);
// DYNET_C_PTR_TO_PTR(Node, dynetNode);
// DYNET_C_VAL_TO_PTR(Node, dynetNode);
// DYNET_C_PTR_TO_PTR(Graph, dynetGraph);
// DYNET_C_PTR_TO_PTR(Initializer, dynetInitializer);
// DYNET_C_PTR_TO_PTR(Model, dynetModel);
// DYNET_C_PTR_TO_PTR(Parameter, dynetParameter);
// DYNET_C_PTR_TO_PTR(Shape, dynetShape);
// DYNET_C_VAL_TO_PTR(Shape, dynetShape);
DYNET_C_PTR_TO_PTR(Dim, dynetDim);
DYNET_C_VAL_TO_PTR(Dim, dynetDim);
// DYNET_C_PTR_TO_PTR(Tensor, dynetTensor);
// DYNET_C_VAL_TO_PTR(Tensor, dynetTensor);
// DYNET_C_PTR_TO_PTR(Optimizer, dynetOptimizer);

template<typename T, typename U>
inline void move_vector_to_array_of_c_ptrs(
    std::vector<T> *src, U **array, std::size_t *size) {
  if (array) {
    if (*size < src->size()) {
      DYNET_C_THROW_ERROR("Size is not enough to move a vector.");
    }
    std::transform(std::make_move_iterator(src->begin()),
                   std::make_move_iterator(src->end()),
                   array,
                   [](T &&x) {
                     return to_c_ptr_from_value(std::forward<T>(x));
                   });
  } else {
    *size = src->size();
  }
}

template<typename T>
inline void copy_vector_to_array(
    const std::vector<T> &src, T *array, std::size_t *size) {
  if (array) {
    if (*size < src.size()) {
      DYNET_C_THROW_ERROR("Size is not enough to copy a vector.");
    }
    std::copy(src.begin(), src.end(), array);
  } else {
    *size = src.size();
  }
}

inline void copy_string_to_array(
    const std::string &str, char *buffer, std::size_t *size) {
  if (buffer) {
    if (*size <= str.length()) {
      DYNET_C_THROW_ERROR("Size is not enough to copy a string.");
    }
    std::strcpy(buffer, str.c_str());
  } else {
    *size = str.length() + 1u;
  }
}

}  // namespace internal

}  // namespace dynet_c

#endif  // DYNET_C_INTERNAL_H_

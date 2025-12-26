#include <IR/ScopeVariable.h>

namespace IR {

llvm::Value *ScopeVariable::getValue(llvm::IRBuilder<llvm::NoFolder> *builder) {

  // If the variable is created by an 'alloca' instruction, then it is
  // actually a pointer to the underlying type. In order to get this value, we
  // need to load it.
  //
  // clang-format off
    // Example:
    //
    //   %something = alloca i32, align 4              ; something is a pointer.
    //   store i32 4, ptr %something, align 4          ; Poiner is used when storing.
    //   ...
    //   %1 add i32 1, 2
    //   %loaded_something = load i32, ptr %something  ; We now need to load it.
    //   %2 = add i32 %1, %loaded_one                  ; Use the loaded value.
    //
  // clang-format on
  if (isAllocaValue) {
    return builder->CreateLoad(underlyingType, variable);
  }

  // Otherwise, the value is already directly stored in the variable. This is
  // the case for parameters for example.
  return variable;
}

llvm::Value *
ScopeVariable::getAddress(llvm::IRBuilder<llvm::NoFolder> *builder) {

  // Similar logic to getValue but the inverse. If the variable was created by
  // 'alloca', then the variable is the already a pointer, so return it.
  if (isAllocaValue) {
    return variable;
  }

  // Otherwise, variable is the value itself. To get the pointer, we
  // have to actually allocate it, store the variable inside of it and then
  // return that.
  auto *valuePointer = builder->CreateAlloca(underlyingType, nullptr);
  builder->CreateStore(variable, valuePointer);

  // Set this variable as the new pointer that was created. This way changes
  // we make to it presists.
  isAllocaValue = true;
  variable = valuePointer;

  return valuePointer;
}

llvm::Value *
ScopeVariable::dereference(llvm::IRBuilder<llvm::NoFolder> *builder,
                           int indirectionStepsToTake) {

  // The current variable is of type pointer. The value of this pointer is a
  // pointer to somewhere else. We want to read what is at the pointed to
  // location. This should be repeated indirectionStepsToTake times.
  // - In order dereference correctly, we have to consider the fact that we
  //   have to load the last pointer we end up at an additional time to get
  //   the value being pointed to.
  // - So we actually need to dereference the 'pointed to' pointer to
  //   get the actual value to return in the end.
  //
  // clang-format off
    // Example:
    //
    // .arda:
    //  int a = 42
    //  int &b = &a
    //  int &&c = &b
    //  return **c
    //
    // .llvm
    //  %a = alloca i32, align 4         ; Declare as %a pointer to an i32.
    //  store i32 42, ptr %a, align 4    ; Store 42 into in to %a.
    //  %b = alloca ptr, align 8         ; Declare %b as a pointer to a pointer.
    //  store ptr %a, ptr %b, align 8    ; Store the pointer %a as value in %b
    //  %c = alloca ptr, align 8         ; Declare %c as a pointer to a pointer.
    //  store ptr %b, ptr %c, align 8    ; Store the pointer %b as value in %c
    //  ...
    //  ; Now we want to return the stored 42 through the pointer %c.
    //  ...
    //  %0 = load ptr, ptr %b, align 8   ; Load the value of %c into %0, this is pointer %b. 
    //  %1 = load ptr, ptr %0, align 8   ; Load the value of %0 into %1, this is pointer %a.
    //  %2 = load i32, ptr %2, align 4   ; Now, load the i32 content of %1 into %2, this is 42. 
    //  ret i32 %2
    //
  // clang-format on

  auto variableToLoad = variable;

  // Perform each indirection step.
  for (int indirectionStep = 1; indirectionStep <= indirectionStepsToTake;
       ++indirectionStep) {

    // Get the value of current pointer and store it as the next variable to
    // load. This is always another pointer.
    variableToLoad = builder->CreateLoad(builder->getPtrTy(), variableToLoad);

    // If we are at the last indireciton step, it means we have reached the
    // value to return.
    if (indirectionStep == indirectionStepsToTake) {

      // Determine the type of the value to return. If we are at the last
      // indirection step possible, then it means we've arrived to the actual
      // value.
      auto typeToLoad = indirectionStep == pointerIndirectionCount
                            ? underlyingType
                            : builder->getPtrTy();

      // Load the value to return from the value of the last pointer.
      variableToLoad = builder->CreateLoad(typeToLoad, variableToLoad);
    }
  }
  return variableToLoad;
}

} // namespace IR
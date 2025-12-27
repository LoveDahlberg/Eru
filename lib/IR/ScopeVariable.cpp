#include <IR/ScopeVariable.h>

namespace IR {

llvm::Value *ScopeVariable::getValue(llvm::IRBuilder<llvm::NoFolder> *builder) {

  // If the variable is created by an 'alloca' instruction, then it is
  // actually a pointer to the underlying type. In order to get this value, we
  // need to load it. The same applies if the 'alloca' was used to create a
  // pointer.
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
    return builder->CreateLoad(
        pointerIndirectionCount == 0 ? underlyingType : builder->getPtrTy(),
        variable);
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
  auto *valuePointer = builder->CreateAlloca(variable->getType(), nullptr);
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

  // When dereferencing, we need the target to be allocated. Call getAddress to
  // create it if needed. If this is not done, parameters cannot be dereferenced
  // correctly.
  getAddress(builder);

  // The current variable is of type pointer. The value of this pointer is a
  // pointer to somewhere else. We want to read what is at the pointed to
  // location. This should be repeated indirectionStepsToTake times.

  auto variableToLoad = variable;

  // Perform each indirection step.
  for (int indirectionStep = 1; indirectionStep <= indirectionStepsToTake;
       ++indirectionStep) {

    // Get the value of current pointer and store it as the next variable to
    // load. This is always another pointer.
    variableToLoad = builder->CreateLoad(builder->getPtrTy(), variableToLoad);
  }
  return variableToLoad;
}

llvm::Value *
ScopeVariable::dereferenceAssignment(llvm::IRBuilder<llvm::NoFolder> *builder,
                                     int indirectionStepsToTake) {
  // The variable we have after the dereference call is a pointer. We are either
  // at a pointer that points to:
  //  1. another pointer.
  //  2. the actual value.
  // Regardless of which, we need to use the current pointer itself in order to
  // use it in an assignment.
  return dereference(builder, indirectionStepsToTake);
}

llvm::Value *
ScopeVariable::dereferenceExpression(llvm::IRBuilder<llvm::NoFolder> *builder,
                                     int indirectionStepsToTake) {

  // The variable we have after the dereference call is a pointer. We are either
  // at a pointer that points to:
  //  1. another pointer.
  //  2. the actual value.
  // Regardless of which, we need to get the value of the current pointer in
  // order to use it in an expression.
  //
  // If we have the following example:
  //
  // clang-format off
  //
  // .arda:
  //  int a = 42
  //  int &b = &a
  //  int &&c = &b
  //  return **c
  //
  // The below IR is generated before this function is called.
  //
  // .llvm
  //  %a = alloca i32, align 4         ; Declare as %a pointer to an i32.
  //  store i32 42, ptr %a, align 4    ; Store 42 into in to %a.
  //  %b = alloca ptr, align 8         ; Declare %b as a pointer to a pointer.
  //  store ptr %a, ptr %b, align 8    ; Store the pointer %a as value in %b
  //  %c = alloca ptr, align 8         ; Declare %c as a pointer to a pointer.
  //  store ptr %b, ptr %c, align 8    ; Store the pointer %b as value in %c
  //
  // clang-format on

  auto variableToLoad = dereference(builder, indirectionStepsToTake);

  // The above call creates the following.
  //
  // clang-format off
  //
  // .llvm
  //  %0 = load ptr, ptr %b, align 8   ; Load the value of %c into %0, this is pointer %b. 
  //  %1 = load ptr, ptr %0, align 8   ; Load the value of %0 into %1, this is pointer %a.
  //  %2 = load i32, ptr %2, align 4   ; Now, load the i32 content of %1 into %2, this is 42. 
  //
  // clang-format on

  // The type to load depends on if the current pointer points to another
  // pointer or the underlying value.
  auto typeToLoad = indirectionStepsToTake == pointerIndirectionCount
                        ? underlyingType
                        : builder->getPtrTy();

  // Perform the load.
  return variableToLoad = builder->CreateLoad(typeToLoad, variableToLoad);

  // The above call creates the following.
  //
  // clang-format off
  //
  // .llvm
  //  %2 = load i32, ptr %2, align 4   ; Now, load the i32 content of %1 into %2, this is 42. 
  //
  // Afterwards, the following is created.
  //
  //  ret i32 %2
  //
  // clang-format on
}

} // namespace IR
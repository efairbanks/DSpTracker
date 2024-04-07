#ifndef INPUT_PROCESSOR_H
#define INPUT_PROCESSOR_H

#include <nds.h>
#include <stdio.h>

class InputProcessor {
public:
  static InputProcessor * getInstance() {
    if(nullptr == instance) {
      instance = new InputProcessor();
    }
    return instance;
  };
  ~InputProcessor() = default;
private:
  static InputProcessor * instance;
  InputProcessor() = default;
  InputProcessor(const InputProcessor&)= delete;
  InputProcessor& operator=(const InputProcessor&)= delete;
};
InputProcessor * InputProcessor::instance = nullptr;

#endif
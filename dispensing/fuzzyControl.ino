#include "src/eFLL/FuzzyRule.h"
#include "src/eFLL/FuzzyComposition.h"
#include "src/eFLL/Fuzzy.h"
#include "src/eFLL/FuzzyRuleConsequent.h"
#include "src/eFLL/FuzzyOutput.h"
#include "src/eFLL/FuzzyInput.h"
#include "src/eFLL/FuzzyIO.h"
#include "src/eFLL/FuzzySet.h"
#include "src/eFLL/FuzzyRuleAntecedent.h"

// Initialize Fuzzy Object
Fuzzy* fuzzy = new Fuzzy();

/* Constants */
// Fill Percentage [ 0 - 100 % ]
const FuzzySet* notFilled = new FuzzySet(0, 0, 20, 40);
const FuzzySet* somewhatFilled = new FuzzySet(20, 65, 65, 90);
const FuzzySet* nearlyFilled = new FuzzySet(80, 87, 87, 95);
const FuzzySet* filled = new FuzzySet(90, 95, 100, 100);

// Cup Height [ mm ]
const FuzzySet* small = new FuzzySet(0, 65, 65, 100);
const FuzzySet* big = new FuzzySet(70, 225, 225, 300);

// Dispense Speed [ 0 - 255 PWM ]
const FuzzySet* zero = new FuzzySet(0, 0, 0, 0); // 0.0
const FuzzySet* slow = new FuzzySet(1, 127, 127, 250); // 0.5
const FuzzySet* fast = new FuzzySet(250, 255, 255, 255); // 1

void fuzzyLogicSetup() {

  // Adding Fill Percentage as Input
  FuzzyInput* fillPercentage = new FuzzyInput(1);
  fillPercentage->addFuzzySet(notFilled);
  fillPercentage->addFuzzySet(somewhatFilled);
  fillPercentage->addFuzzySet(nearlyFilled);
  fillPercentage->addFuzzySet(filled);

  // Adding Cup Height as Input
  FuzzyInput* cupHeight = new FuzzyInput(2);
  cupHeight->addFuzzySet(small);
  cupHeight->addFuzzySet(big);

  // Adding Dispense Speed as Output
  FuzzyOutput* dispenseSpeed = new FuzzyOutput(1);
  dispenseSpeed->addFuzzySet(zero);
  dispenseSpeed->addFuzzySet(slow);
  dispenseSpeed->addFuzzySet(fast);

  // Attach Inputs/outputs to fuzzy object
  fuzzy->addFuzzyInput(fillPercentage);
  fuzzy->addFuzzyInput(cupHeight);
  //fuzzy->addFuzzyInput(error);
  //fuzzy->addFuzzyInput(cupOpening);
  fuzzy->addFuzzyOutput(dispenseSpeed);

  /* Building Fuzzy Rules */
   
  // Rule # 1: If cup is filled, stop pouring
  
  FuzzyRuleAntecedent* ifFilled = new FuzzyRuleAntecedent();
  ifFilled->joinSingle(filled);

  FuzzyRuleConsequent* thenStopFilling = new FuzzyRuleConsequent();
  thenStopFilling->addOutput(zero);

  // Rule # 2: If cup small AND not filled OR nearly filled, pour slowly
  
  FuzzyRuleAntecedent* ifSmallAndNotFilled = new FuzzyRuleAntecedent();
  ifSmallAndNotFilled->joinWithAND(small, notFilled);
  
  FuzzyRuleAntecedent* ifSlowPouringConditions = new FuzzyRuleAntecedent();
  ifSlowPouringConditions->joinWithOR(ifSmallAndNotFilled, nearlyFilled);

  FuzzyRuleConsequent* thenSlowlyFill = new FuzzyRuleConsequent();
  thenSlowlyFill->addOutput(slow);

  // Rule #3: If cup big and not filled or somewhatFilled, pour fast
  
  FuzzyRuleAntecedent* ifBigAndNotFilled = new FuzzyRuleAntecedent();
  ifBigAndNotFilled->joinWithAND(big, notFilled);

  FuzzyRuleAntecedent* ifFastPouringConditions = new FuzzyRuleAntecedent();
  ifFastPouringConditions->joinWithOR(ifBigAndNotFilled, somewhatFilled);

  FuzzyRuleConsequent* thenFastFill = new FuzzyRuleConsequent();
  thenFastFill->addOutput(fast);

  // Add the Rules to the fuzzy controller
  
  FuzzyRule* fuzzyRule1 = new FuzzyRule(1, ifFilled, thenStopFilling);
  fuzzy->addFuzzyRule(fuzzyRule1);
  FuzzyRule* fuzzyRule2 = new FuzzyRule(2, ifSlowPouringConditions, thenSlowlyFill);
  fuzzy->addFuzzyRule(fuzzyRule2);
  FuzzyRule* fuzzyRule3 = new FuzzyRule(3, ifFastPouringConditions, thenFastFill);
  fuzzy->addFuzzyRule(fuzzyRule3);
}

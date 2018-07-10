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
const FuzzySet* somewhatFilled = new FuzzySet(10, 40, 40, 70);
const FuzzySet* mostlyFilled = new FuzzySet(60, 75, 75, 90);
const FuzzySet* nearlyFilled = new FuzzySet(80, 87, 87, 95);
const FuzzySet* filled = new FuzzySet(90, 95, 100, 100);

// Cup Height [ mm ]
const FuzzySet* small = new FuzzySet(30, 65, 65, 100);
const FuzzySet* medium = new FuzzySet(70, 120, 120, 170);
const FuzzySet* large = new FuzzySet(150, 225, 225, 300);

// Cup Opening ( Diameter between two rims ) [ steps ]
// TODO(Rebecca): Determine when have system what these values need to be
// Note: Mockup Values for now.... unsure if needed
//const FuzzySet* tight = new FuzzySet(20, 30, 30, 40);
//const FuzzySet* regular = new FuzzySet(35, 50, 50, 90);
//const FuzzySet* wide = new FuzzySet(60, 105, 105, 150);

// Dispense Speed [ 0 - 255 PWM ]
const FuzzySet* stopped = new FuzzySet(0, 0, 0, 0); // 0.0
const FuzzySet* slow = new FuzzySet(127, 127, 127, 127); // 0.5
const FuzzySet* fast = new FuzzySet(255, 255, 255, 255); // 1

void fuzzyLogicSetup() {

  // Adding Fill Percentage as Input
  FuzzyInput* fillPercentage = new FuzzyInput(1);
  fillPercentage->addFuzzySet(notFilled);
  fillPercentage->addFuzzySet(somewhatFilled);
  fillPercentage->addFuzzySet(mostlyFilled);
  fillPercentage->addFuzzySet(nearlyFilled);
  fillPercentage->addFuzzySet(filled);

  // Adding Cup Height as Input
  FuzzyInput* cupHeight = new FuzzyInput(2);
  cupHeight->addFuzzySet(small);
  cupHeight->addFuzzySet(medium);
  cupHeight->addFuzzySet(large);

  // Adding Cup Opening as Input
  //FuzzyInput* cupOpening = new FuzzyInput(3);
  //cupOpening->addFuzzySet(tight);
  //cupOpening->addFuzzySet(regular);
  //cupOpening->addFuzzySet(wide);

  // Adding Dispense Speed as Output
  FuzzyOutput* dispenseSpeed = new FuzzyOutput(1);
  dispenseSpeed->addFuzzySet(stopped);
  dispenseSpeed->addFuzzySet(slow);
  dispenseSpeed->addFuzzySet(fast);

  // Attach Inputs/outputs to fuzzy object
  fuzzy->addFuzzyInput(fillPercentage);
  fuzzy->addFuzzyInput(cupHeight);
  //fuzzy->addFuzzyInput(cupOpening);
  fuzzy->addFuzzyOutput(dispenseSpeed);

   /* Building Fuzzy Rules */
  // Rule # 1: If cup is filled, stop pouring
  
  FuzzyRuleAntecedent* ifFilled = new FuzzyRuleAntecedent();
  ifFilled->joinSingle(filled);

  FuzzyRuleConsequent* thenStopFilling = new FuzzyRuleConsequent();
  thenStopFilling->addOutput(stopped);

  FuzzyRule* fuzzyRule1 = new FuzzyRule(1, ifFilled, thenStopFilling);
  fuzzy->addFuzzyRule(fuzzyRule1);

  // Rule # 2: If cup small AND not filled OR nearly filled, pour slowly
  
  FuzzyRuleAntecedent* ifCupSmallAndNotFilled = new FuzzyRuleAntecedent();
  ifCupSmallAndNotFilled->joinWithAND(small, notFilled);
  FuzzyRuleAntecedent* ifNearlyFilled = new FuzzyRuleAntecedent();
  ifNearlyFilled->joinSingle(nearlyFilled);
  FuzzyRuleAntecedent* ifCupSmallAndNotFilledOrNearlyFilled = new FuzzyRuleAntecedent();
  ifCupSmallAndNotFilledOrNearlyFilled->joinWithOR(ifCupSmallAndNotFilled, ifNearlyFilled);

  FuzzyRuleConsequent* thenFillSlowly = new FuzzyRuleConsequent();
  thenFillSlowly->addOutput(slow);

  FuzzyRule* fuzzyRule2 = new FuzzyRule(2, ifCupSmallAndNotFilledOrNearlyFilled, thenFillSlowly);
  fuzzy->addFuzzyRule(fuzzyRule2);

  // Rule #3: If cup medium or large and not filled or somewhatFilled or mostlyFilled, pour fast
  // Note: Basically fast speed for all remaining cases
  
  FuzzyRuleAntecedent* ifCupMediumOrLarge = new FuzzyRuleAntecedent();
  ifCupMediumOrLarge->joinWithOR(medium, large);
  FuzzyRuleAntecedent* ifNotFilled = new FuzzyRuleAntecedent();
  ifNotFilled->joinSingle(notFilled);
  FuzzyRuleAntecedent* ifCupMediumOrLargeAndNotFilled = new FuzzyRuleAntecedent();
  ifCupMediumOrLargeAndNotFilled->joinWithAND(ifCupMediumOrLarge, ifNotFilled);
  FuzzyRuleAntecedent* ifSomewhatFilledOrMostlyFilled = new FuzzyRuleAntecedent();
  ifSomewhatFilledOrMostlyFilled->joinWithOR(somewhatFilled, mostlyFilled);
  FuzzyRuleAntecedent* ifRemainingCases = new FuzzyRuleAntecedent();
  ifRemainingCases->joinWithOR(ifCupMediumOrLargeAndNotFilled, ifSomewhatFilledOrMostlyFilled);

  FuzzyRuleConsequent* thenPourFast = new FuzzyRuleConsequent();
  thenPourFast->addOutput(fast);

  FuzzyRule* fuzzyRule3 = new FuzzyRule(3, ifRemainingCases, thenPourFast);
  fuzzy->addFuzzyRule(fuzzyRule3);
}

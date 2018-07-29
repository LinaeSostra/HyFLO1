#ifndef ENUMS_H
#define ENUMS_H

typedef enum FillSpeed {
  zero,
  half,
  full
} FillSpeed;

typedef enum FillAmount {
  noFill,
  halfFill,
  threeQuartersFill,
  fullFill
} FillAmount;

typedef enum CupHeight {
  noHeight,
  small,
  medium,
  large
} CupHeight;

#endif

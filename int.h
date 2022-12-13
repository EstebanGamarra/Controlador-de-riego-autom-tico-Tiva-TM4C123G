/*
;*********** DisableInterrupts ***************
; disable interrupts
; inputs:  none
; outputs: none
*/
void DisableInterrupts (void);

/*
;*********** EnableInterrupts ***************
; disable interrupts
; inputs:  none
; outputs: none
*/
void EnableInterrupts (void);

/*
;*********** StartCritical ************************
; make a copy of previous I bit, disable interrupts
; inputs:  none
; outputs: previous I bit
*/
int StartCritical (void);

/*
;*********** EndCritical ************************
; using the copy of previous I bit, restore I bit to previous value
; inputs:  previous I bit
; outputs: none
*/
void EndCritical (int previousI);

/*
;*********** WaitForInterrupt ************************
; go to low power mode while waiting for the next interrupt
; inputs:  none
; outputs: none
*/
void WaitForInterrupt (void);

#include "riscv_test.h"
RVTEST_RV64S

.global enter_s
enter_s:

	RVTEST_CODE_BEGIN
	j s_entry


#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <unistd.h>
#include <Neoeter/Platform/Base/Timer.h>

int main()
{
	printf("[TEST] Starting CTimer and ELTimer tests...\n");

	// 1. ELTimer
	int32_t initOk = ELTimer_Init();
	assert(initOk == TRUE);

	uint32_t t1 = ELTimer_GetMSec();
	usleep(20000); // Sleep ~20ms
	uint32_t t2 = ELTimer_GetMSec();
	assert(t2 >= t1);
	printf("  [PASS] ELTimer advanced: %u ms -> %u ms (delta: %u ms)\n", t1, t2, t2 - t1);

	// 2. CTimer singleton instance
	CTimer timer;
	usleep(5000); // Ensure time > 0 before first frame
	timer.Advance();
	usleep(20000); // Sleep ~20ms
	timer.Advance();

	float elapsedSec = timer.GetElapsedSecond();
	uint32_t elapsedMs = timer.GetElapsedMilliecond();
	printf("DEBUG: elapsedSec=%f, elapsedMs=%u\n", elapsedSec, elapsedMs);
	assert(elapsedSec >= 0.010f && elapsedSec <= 0.100f);
	printf("  [PASS] CTimer GetElapsedSecond(): %.4f s\n", elapsedSec);

	printf("[TEST] CTimer tests PASSED successfully.\n");
	return 0;
}

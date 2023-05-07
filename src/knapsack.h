#pragma once

typedef void CBSENDRESULT(int num, int* res, __int64 size, __int64 maxsize);

/// <summary>
/// Initialize knapsack solver
/// </summary>
/// <param name="nums">Numbers to process</param>
/// <param name="nums_n">Amount of numbers</param>
/// <param name="maxsum">Maximum allowed sum</param>
/// <param name="res">Result with indexes of nums[] and best sum as a last element</param>
/// <param name="stop">Emergency stop flag. if set to 1, the calculation will stop immediately</param>
/// <param name="slack">Slack of the maxsum, so the valid values are falling in the range [maxsum - slack; maxsum]</param>
/// <param name="fnSendRes">Callback function, called when a new result is found, could be NULL</param>
/// <param name="maxfiles">Limit number of elements in the result</param>
void Ks_Init(const unsigned __int64 nums[], int nums_n, unsigned __int64 maxsum, int res[],
			 int* stop, unsigned __int64 slack, CBSENDRESULT* fnSendRes, int maxfiles);

/// <summary>
/// Starts knapsack solver
/// </summary>
/// <param name="bestsum">Returns the sum of found numbers, could be NULL</param>
/// <returns>Number of elements used in res</returns>
int Ks_Start(__int64* bestsum);

/// <summary>Free used resources </summary>
void Ks_Done();
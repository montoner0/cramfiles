#include <assert.h>
#include "knapsack.h"

typedef struct
{
	int n;
	const unsigned __int64* nums;
	unsigned __int64 maxsum;
	unsigned __int64 bestsum;
	unsigned __int64 cursum;
	int* preres;
	int* res;
	int count;
	int bestcount;
	int* stop;
	unsigned __int64 slack;
	int used_nums;
	int reclvl;
	CBSENDRESULT* fnSendRes;
	int maxfiles;
} KS, * PKS;

static KS s;

void Ks_Init(const unsigned __int64 nums[], int nums_n, unsigned __int64 maxsum,
			 int res[], int* stop, unsigned __int64 slack, CBSENDRESULT* fnSendRes, int maxres)
{

	assert(nums && res);

	s.n = nums_n;
	s.nums = nums;
	s.maxsum = maxsum;
	s.res = res;
	s.preres = new int[nums_n];
	s.cursum = s.count = s.used_nums = s.reclvl = 0;
	s.stop = stop;
	s.slack = slack;
	s.fnSendRes = fnSendRes;
	s.maxfiles = maxres - 1;

	for (int i = 0; i < nums_n; i++) s.preres[i] = -1;

}

static void _Ks(int used_nums_count, int reclevel)
{

	int End = s.n - 1;
	//static int brk=0;

	for (int i = used_nums_count; i < s.n && !(*s.stop) /*&& !brk*//*&& (s.maxsum-s.bestsum)>s.eps*/; i++) {
		s.cursum += s.nums[i];

		if (s.cursum <= s.maxsum) {
			s.preres[reclevel] = i;
			s.count++;

			if ((reclevel < s.maxfiles) && (i < End))
				_Ks(i + 1, reclevel + 1);
			// else
			//    if(reclevel==4 && i==End && s.cursum<s.maxsum && s.bestsum<s.maxsum)
			//       *s.stop=1; // overall size less than maxsize

			if (s.cursum > s.bestsum) {
				s.bestsum = s.cursum;
				s.bestcount = s.count;
				s.used_nums = i + 1;
				s.reclvl = reclevel;
				for (int k = s.n - 1; k >= 0; k--)
					s.res[k] = s.preres[k];
				if ((s.maxsum - s.bestsum) <= s.slack) {
					if (s.fnSendRes) {
						(*s.fnSendRes)(s.bestcount, s.res, s.bestsum, s.maxsum);
					} else {
						*s.stop = 1;
					}
				}
			} else {
				if ((s.maxsum - s.cursum) <= s.slack && s.fnSendRes) {
					(*s.fnSendRes)(s.count, s.preres, s.cursum, s.maxsum);
				}
			}

			s.count--;
			s.preres[reclevel] = -1;
		}

		s.cursum -= s.nums[i];
	}
}

int Ks_Start(__int64* bestsum)
{

	s.bestsum = s.bestcount = 0;

	_Ks(s.used_nums, s.reclvl);
	if (bestsum)
		*bestsum = s.bestsum;

	return s.bestcount;
}

void Ks_Done()
{

	delete[] s.preres;
}

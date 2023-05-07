#include <stdio.h>
#include <stdlib.h>

#include <Windows.h>
#include <assert.h>
#include <tchar.h>
#include <string>
#include <vector>
#include "knapsack.h"
#include <clocale>

using namespace std;

typedef basic_string< TCHAR, char_traits<TCHAR>, allocator<TCHAR> > tstring;

typedef struct
{
	__int64 fsize;
	tstring filename;
} FILES, * PFILES;

typedef vector<FILES> FILESLIST;

FILESLIST g_files;

int num;
__int64 g_size;
int stopcontrol;
//////////////////////////////////////////////////////////////////////////
inline int chricmp(const TCHAR c1, const TCHAR c2)
{
	return _totlower(c1) == _totlower(c2);
}
//////////////////////////////////////////////////////////////////////////
int wildcmp(PTCHAR wild, PTCHAR string, BOOL bCaseSens)
{
	// Originally written by Jack Handy - jakkhandy@hotmail.com
	PTCHAR cp = NULL, mp = NULL;

	assert(wild && string);

	while ((*string) && (*wild != _T('*'))) {
		if ((bCaseSens ? *wild != *string : !chricmp(*wild, *string)) && (*wild != _T('?'))) {
			return 0;
		}
		wild++;
		string++;
	}

	while (*string) {
		if (*wild == _T('*')) {
			if (!*++wild) {
				return 1;
			}
			mp = wild;
			cp = string + 1;
		} else
			if ((bCaseSens ? *wild == *string : chricmp(*wild, *string))/*(*wild == *string)*/ || (*wild == _T('?'))) {
				wild++;
				string++;
			} else {
				wild = mp;
				string = cp++;
			}
	}

	while (*wild == _T('*')) {
		wild++;
	}
	return !*wild;
}
//////////////////////////////////////////////////////////////////////////
BOOL IsInWildcard(PTCHAR str, PTCHAR wcs)
{

	PTCHAR tok = NULL, context;

	tok = _tcstok_s(wcs, _T(";"), &context);
	while (tok) {
		if (wildcmp(tok, str, FALSE))
			return TRUE;

		tok = _tcstok_s(NULL, _T(";"), &context);
		if (tok) tok[-1] = _T(';');
	}

	return FALSE;
}
//////////////////////////////////////////////////////////////////////////
DWORD CalcDirSize(const PTCHAR dirname, BOOL bRecursive, __int64* DirSize, const PTCHAR exclude)
{

	WIN32_FIND_DATA FileData;
	HANDLE hSearch;
	DWORD LastErr = 0;
	static int counter = 0;

	if (counter == 0) *DirSize = 0;
	counter++;

	if (!SetCurrentDirectory(dirname))
		return GetLastError();

	hSearch = FindFirstFile(_T("*.*"), &FileData);
	if (hSearch == INVALID_HANDLE_VALUE) {
		LastErr = GetLastError();
		SetCurrentDirectory(_T(".."));
		return LastErr;
	}

	do {
		if (!((FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
			  ((_tcscmp(FileData.cFileName, _T("..")) == 0) || (_tcscmp(FileData.cFileName, _T(".")) == 0))
			  ) && !IsInWildcard(FileData.cFileName, exclude)/*wildcmp(exclude,FileData.cFileName,FALSE)*/) {



			if ((FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && bRecursive)
				CalcDirSize(FileData.cFileName, bRecursive, DirSize, exclude);
			else
				*DirSize += ((__int64)FileData.nFileSizeHigh << 32) | FileData.nFileSizeLow;
		}

		if (!FindNextFile(hSearch, &FileData))
			LastErr = GetLastError();

	} while (LastErr == 0);

	FindClose(hSearch);

	SetCurrentDirectory(_T(".."));

	counter--;
	return LastErr;
}
//////////////////////////////////////////////////////////////////////////

BOOL WINAPI CtrlCHandler(DWORD dwCtrlType)
{
	//    switch(dwCtrlType){
	//    case CTRL_C_EVENT:
	//    case CTRL_BREAK_EVENT:{
	_putts(_T("\nPremature end, results could be not optimal..."));
	stopcontrol = 1;
	//       break;
	//    }
	//
	//    default: return FALSE;

	//    }
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

void ParseFiles(PTCHAR fn, PTCHAR exclude)
{

	BOOL bRecurs, bWildcard = FALSE;
	WIN32_FIND_DATA fd;
	HANDLE hFind;

	bRecurs = fn[_tcslen(fn) - 1] != _T('\\');  // last slash == no recursion
	if (!bRecurs)
		fn[_tcslen(fn) - 1] = 0;

	TCHAR* c = _tcsrchr(fn, _T('\\'));
	if (c)
		bWildcard = _tcspbrk(c + 1, _T("*?")) != NULL;

	hFind = FindFirstFile(fn, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		DWORD LastErr = 0;
		do {
			if (_tcscmp(fd.cFileName, _T(".")) != 0 && _tcscmp(fd.cFileName, _T("..")) != 0 && !((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !bRecurs)) {
				FILES f;

				if (bWildcard) {
					*(c + 1) = 0;
					f.filename = fn;
					f.filename += fd.cFileName;
				} else
					f.filename = fn;

				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					CalcDirSize((PTCHAR)f.filename.c_str(), bRecurs, &f.fsize, exclude);
				else
					f.fsize = ((__int64)fd.nFileSizeHigh << 32) | fd.nFileSizeLow;

				g_files.push_back(f);
			}

			if (!FindNextFile(hFind, &fd))
				LastErr = GetLastError();

		} while (LastErr == 0);

		FindClose(hFind);
	} else {
		_tprintf(_T("WARNING! %s is not found!\n"), fn);
	}
}

//////////////////////////////////////////////////////////////////////////

void ShowRes(int num, int res[], __int64 size, __int64 maxsize)
{

	if (num) {
		_tprintf(_T("\n\nFound %d file%s, size = %I64d (%I64d)\n\n"), num, num > 1 ? _T("s") : _T(""), size, size - maxsize);

		for (int i = 0; i < num; i++)
			if (res[i] >= 0) _tprintf(_T("%12I64d \t%s\n"), g_files[res[i]].fsize, g_files[res[i]].filename.c_str());
	}
}

//////////////////////////////////////////////////////////////////////////
// "k","m","g" means *10^[3,6,9]
// "ki","mi","gi" means *2^[10,20,30]
__int64 GetKMGSize(PTCHAR s, _locale_t locl)
{

	DWORD multipl = 1, thousand;
	PTCHAR c;

	assert(s);

	c = &s[_tcslen(s) - 1];
	if (_totlower(*c) == _T('i')) {
		thousand = 1024;
		*c = 0;
	} else
		thousand = 1000;


	switch (_totlower(s[_tcslen(s) - 1])) {
		case 'k':multipl = thousand; break;
		case 'm':multipl = thousand * thousand; break;
		case 'g':multipl = thousand * thousand * thousand; break;
	}

	return __int64(_tcstod_l(s, NULL, locl) * multipl + .5);
}
//////////////////////////////////////////////////////////////////////////

int _tmain(int argc, _TCHAR* argv[])
{

	FILE* f;
	TCHAR s[32767], exclude[MAX_PATH];
	BOOL bFullSearch = FALSE;
	_locale_t locl;
	unsigned __int64 slack = 0;
	int maxfiles = 0;

	locl = _get_current_locale();

	_tsetlocale(LC_ALL, _T(".ACP"));

	if (argc < 3) {
		_putts(_T("Usage: cramfiles <filemask|dirname[\\]|@listfile> <targetsize[k|m|g][i]>"));
		_putts(_T("                 [-s <slack_size>[k|m|g][i]] [-f] [-x <exclude_filedir>] [-m <max_files_count>]\n"));
		_putts(_T("   filemask\t path and mask of files to analyze"));
		_putts(_T("   dirname[\\]\t path to a folder. It will be treated as a single entry."));
		_putts(_T("                 If a trailing backslash is specified then the folder won't be recursed"));
		_putts(_T("   @listfile\t path to a list file consisting of file masks and/or paths to folders"));
		_putts(_T("   targetsize\t taget sum of file sizes. "));
		_putts(_T("   \t\t Could be suffixed with k, m or g for kilo/mega/giga. Additional suffix i for kibi/mebi/gibi"));
		_putts(_T("   -s\t\t slack of the target sum, so the valid values are considered in the range [target - slack; target]"));
		_putts(_T("   -f\t\t full search mode. Will try to find all possible solutions, not just the first one"));
		_putts(_T("   -x\t\t filemask/path to exclude from analyse"));
		_putts(_T("   -m\t\t maximum number of files allowed within the result"));
		return 255;
	}

	for (int i = 3; i < argc; i++) {
		if (argv[i][0] == _T('-')) {
			switch (_totupper(argv[i][1])) {
				case _T('F'):
					bFullSearch = TRUE;
					break;
				case _T('X'):
					i++;
					_tcscpy_s(exclude, _countof(exclude), argv[i]);
					break;
				case _T('S'):
					i++;
					slack = GetKMGSize(argv[i], locl);
					break;
				case _T('M'):
					i++;
					maxfiles = _tcstol(argv[i], NULL, 10);
					break;
				default:
					_tprintf(_T("Warning! Unrecognized switch \"%s\", ignored\n"), argv[i]);
			}
		} else
			_tprintf(_T("Warning! Unrecognized parameter \"%s\", ignored\n"), argv[i]);
	}

	_putts(_T("Files collecting...\n"));
	if (argv[1][0] == '@') {
		if (_tfopen_s(&f, &argv[1][1], _T("rt"))) {
			_tprintf(_T("ERROR: opening file '%s' has failed\n"), &argv[1][1]);
			return 2;
		}
		while (!feof(f)) {
			if (_ftscanf_s(f, _T("%[^\r\n]%*[\r\n]"), s, _countof(s)) >= 1) {
				ParseFiles(s, exclude);
			}
		}
	} else {
		ParseFiles(argv[1], exclude);
	}
	_tprintf(_T("%d files and/or directories is collected\n"), g_files.size());

	if (g_files.size() > 0) {
		unsigned __int64* pfsizes = new unsigned __int64[g_files.size()];
		int* res = new int[g_files.size()];
		unsigned __int64 targetsize = GetKMGSize(argv[2], locl);

		if (maxfiles == 0) maxfiles = (int)g_files.size();

		if (!SetConsoleCtrlHandler(CtrlCHandler, TRUE))
			_putts(_T("ERROR: Could not set control-c handler"));

		_tprintf(_T("\nTarget size = %I64d bytes\nSlack = %I64d bytes\n\nStart searching...\n"), targetsize, slack);

		if (bFullSearch) {
			_putts(_T("\nFull search mode. This may take a long time, be patient. Press Ctrl-C to stop."));

			for (unsigned int i = 0; i < g_files.size(); i++)
				pfsizes[i] = g_files[i].fsize;

			Ks_Init(pfsizes, (int)g_files.size(), targetsize, res, &stopcontrol, slack, ShowRes, maxfiles);

			stopcontrol = 0;

			//do{

			num = Ks_Start(&g_size);

			if (targetsize - g_size > slack) {
				_putts(_T("\nThe best combination isn't found. The best one so far:"));
				Beep(250, 130);
				ShowRes(num, res, g_size, targetsize);
			}

			//}while (num!=0 && !stopcontrol);

			Ks_Done();

		} else {
			do {
				int i = 0;
				for (FILESLIST::iterator it = g_files.begin(); it != g_files.end(); it++, i++)
					pfsizes[i] = it->fsize;

				//for (int i=0;i<files.size();i++)
				//   pfsizes[i]=files.at(i).fsize;

				stopcontrol = 0;
				Ks_Init(pfsizes, (int)g_files.size(), targetsize, res, &stopcontrol, slack, NULL, maxfiles);

				num = Ks_Start(&g_size);

				Ks_Done();

				ShowRes(num, res, g_size, targetsize);

				for (int i = num - 1; i >= 0; i--) // reversed order is important because of elements shift
					g_files.erase(g_files.begin() + res[i]);

			} while (g_files.size() > 0 && num > 0);

			num = (int)g_files.size();
			if (num > 0) {
				_tprintf(_T("\n\n%d file%s unmatched\n\n"), num, num > 1 ? _T("s") : _T(""));
				for (int i = 0; i < num; i++)
					_tprintf(_T("%12I64d \t%s\n"), g_files[i].fsize, g_files[i].filename.c_str());
			}
		}
		SetConsoleCtrlHandler(CtrlCHandler, FALSE);

		delete[] pfsizes;
		delete[] res;
	} else {
		_putts(_T("No files - nothing to do!"));
	}

	_free_locale(locl);

	return 0;
}
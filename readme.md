# Cramfiles

A simple console app which helps to distribute a set of files into subsets of a specified size. E. g. when you want to backup files to a DVD and don't want to lose space as unused or manually search a proper combination of the files.

NB. It only makes a list, you'll need to do all needed actions to the files yourself.

## Usage

`cramfiles <filemask|dirname[\]|@listfile> <targetsize[k|m|g][i]>
 [-s <slack_size>[k|m|g][i]] [-f] [-x <exclude_filedir>] [-m <max_files_count>]`

   

| Argument     | Meaning                                                                                                                                                           |
| ------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `filemask`   | Path and mask of files to analyze                                                                                                                                 |
| `dirname[\]` | Path to a folder. It will be treated as a single entry. If a trailing backslash is specified then the folder won't be recursed                                    |
| `@listfile`  | Path to a list file consisting of file masks and/or paths to folders                                                                                              |
| `targetsize` | Taget sum of file sizes. Could be suffixed with `k`, `m` or `g` for kilo/mega/giga. Additional suffix `i` for kibi/mebi/gibi                                      |
| `-s`         | Slack of the target sum, so the valid values are considered in the range *[target - slack; target]*. Without the slack it will try to fit exact to the target sum |
| `-f`         | Full search mode. Will try to find all possible solutions, not just the first one                                                                                 |
| `-x`         | Filemask or path to exclude from analyse                                                                                                                          |
| `-m`         | Maximum number of files allowed within the result                                                                                                                 |

## Example

`cramfiles c:\myfamilyphotos\*.jpg 4.7g -s 1m` - will make a list of all .jpg files which fit a DVD
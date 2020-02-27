# Process-Monitoring
Simple Process Monitoring Tool

# Usage
pmcli usage:

  -h [ --help ]                produce help message
  -v [ --version ]             print version string
  -o [ --output ]              output file name
  -i [ --interval ]            interval in ms (default 60000)
  -p [ --process-id ]          monitoring process id (multiple separated by ,)
  -n [ --process-name ]        monitoring process name (multiple separated by ;)
  -t [ --type ]                memory type (see list below)

Types

  pfc: Page fault count
  pwss: Peak working set size
  wss: Working set size (default type)
  qpppu: Quota peak paged pool usage
  qppu: Quota paged pool usage
  qpnppu: Quota peak non paged pool usage
  qnppu: Quota non paged pool usage
  pfu: Page file usage
  ppfu: Peak page file usage

Examples:

  pmcli --process-id 1234,5678
  pmcli --process-name a.exe;b.exe;pmcli
  pmcli  --process-id 1234,5678 --process-name a.exe;b.exe;pmcli

# Process-Monitoring
Simple Process Monitoring Tool

# Usage

<dl>
  <dt>pmcli usage</dt>
  <dd>-h [ --help ]                produce help message</dd>
  <dd>-v [ --version ]             print version string</dd>
  <dd>-o [ --output ]              output file name</dd>
  <dd>-i [ --interval ]            interval in ms (default 60000)</dd>
  <dd>-p [ --process-id ]          monitoring process id (multiple separated by ,)</dd>
  <dd>-n [ --process-name ]        monitoring process name (multiple separated by ;)</dd>
  <dd>-t [ --type ]                memory type (see list below)</dd>

  <dt>Types</dt>
  <dd>pfc: Page fault count</dd>
  <dd>pwss: Peak working set size</dd>
  <dd>wss: Working set size (default type)</dd>
  <dd>qpppu: Quota peak paged pool usage</dd>
  <dd>qppu: Quota paged pool usage</dd>
  <dd>qpnppu: Quota peak non paged pool usage</dd>
  <dd>qnppu: Quota non paged pool usage</dd>
  <dd>pfu: Page file usage</dd>
  <dd>ppfu: Peak page file usage</dd>

  <dt>Examples:</dt>
  <dd>pmcli --process-id 1234,5678</dd>
  <dd>pmcli --process-name a.exe;b.exe;pmcli</dd>
  <dd>pmcli  --process-id 1234,5678 --process-name a.exe;b.exe;pmcli</dd>
</dl>

# Release dependencies
Visual C Redistributable 2019

# To do
Nix support
# ---------------------------------------------------------------------------
# Prove that replacing bare rule numbers with named constants did not change a
# single reported id.
#
# Every validation finding is filed under a rule id: the rule number plus the
# base its severity adds. Replacing the number with a constant must leave that
# id untouched. This script resolves every reporting call to the id it emits,
# in file order, and compares the result against the same file at a chosen
# revision. Two equal sequences mean no rule was renumbered, none was dropped
# and none was added.
#
# Usage:
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\rule-id-audit.ps1 `
#              -Path <source> -Constants <source>[,<source>] [-Revision HEAD] [-List]
#
# -Path       the source whose reporting calls are audited.
# -Constants  the sources declaring the rule constants and their numbers.
# -Revision   the revision to compare against. Defaults to HEAD.
# -List       print the resolved sequence instead of comparing.
#
# Exit code 0 when the sequences match, 1 when they differ. A call whose rule is
# chosen at run time is compared by its own text instead of by a number.
# ---------------------------------------------------------------------------

[CmdletBinding()]
param(
      [Parameter(Mandatory = $true)] [string] $Path
    , [Parameter(Mandatory = $true)] [string[]] $Constants
    , [string] $Revision = 'HEAD'
    , [switch] $List
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = 'Stop'

$WARNING_BASE     = 100
$INFORMATION_BASE = 200

function Get-ConstantValues
{
    # Every rule constant and the number it carries, including one declared as a
    # reference to another.
    param([string] $Text)

    $values = @{}
    foreach ($hit in [regex]::Matches($Text, '(?:static\s+)?(?:final\s+)?(?:constexpr\s+)?int\s+([A-Z][A-Z_0-9]{2,})\s*(?:=|\{)\s*(\d+)'))
    {
        $values[$hit.Groups[1].Value] = [int] $hit.Groups[2].Value
    }

    return $values
}

function Add-Aliases
{
    # A constant written as a reference to another one carries the same number.
    param([hashtable] $Values, [string] $Text)

    for ($pass = 0; $pass -lt 4; ++$pass)
    {
        $added = 0
        foreach ($hit in [regex]::Matches($Text, 'int\s+([A-Z][A-Z_0-9]{2,})\s*=\s*(?:[A-Za-z_][A-Za-z_0-9]*\s*[.:]{1,2}\s*)?([A-Z][A-Z_0-9]{2,})\s*;'))
        {
            $alias  = $hit.Groups[1].Value
            $target = $hit.Groups[2].Value
            if ($Values.ContainsKey($target) -and (-not $Values.ContainsKey($alias)))
            {
                $Values[$alias] = $Values[$target]
                ++$added
            }
        }

        if ($added -eq 0) { break }
    }
}

function Get-FirstArgument
{
    # The first top level argument of the call whose opening parenthesis is at $Open.
    param([string] $Text, [int] $Open)

    $depth = 0
    for ($i = $Open; $i -lt $Text.Length; ++$i)
    {
        $ch = $Text[$i]
        if ($ch -eq '(') { ++$depth; continue }

        if ($ch -eq ')')
        {
            --$depth
            if ($depth -eq 0) { return $Text.Substring($Open + 1, $i - $Open - 1).Trim() }

            continue
        }

        if (($ch -eq ',') -and ($depth -eq 1)) { return $Text.Substring($Open + 1, $i - $Open - 1).Trim() }
    }

    return $null
}

function Resolve-RuleId
{
    # The id an argument emits: a number, a constant, or a constant wrapped in a
    # helper that adds the band of its severity.
    param([string] $Argument, [hashtable] $Values)

    $text = ($Argument -replace '\s+', ' ').Trim()
    if ($text -match '^\d+$') { return [int] $text }

    $wrapper = [regex]::Match($text, '^(?:[A-Za-z_][A-Za-z_0-9]*\s*[.:]{1,2}\s*)?(warning|warningRule|information|informationRule)\s*\((.*)\)$')
    if ($wrapper.Success)
    {
        $inner = Resolve-RuleId -Argument $wrapper.Groups[2].Value -Values $Values
        if ($null -eq $inner) { return $null }

        $base = $(if ($wrapper.Groups[1].Value -like 'warning*') { $WARNING_BASE } else { $INFORMATION_BASE })
        return ($inner + $base)
    }

    $name = [regex]::Match($text, '^(?:[A-Za-z_][A-Za-z_0-9]*\s*[.:]{1,2}\s*)?([A-Z][A-Z_0-9]{2,})$')
    if ($name.Success -and $Values.ContainsKey($name.Groups[1].Value)) { return $Values[$name.Groups[1].Value] }

    return $null
}

function Get-ReportedIds
{
    # Every reporting call in the source, in file order. A call naming a rule gives the id it
    # emits. A call whose rule is decided at run time gives its own text, so the comparison
    # still holds it to being unchanged.
    param([string] $Text, [hashtable] $Values, [ref] $Dynamic)

    $ids = @()
    foreach ($hit in [regex]::Matches($Text, '(?<![A-Za-z0-9_])report(?:Skipped|Warning|Error|Information)\s*\('))
    {
        $open = $Text.IndexOf('(', $hit.Index)
        $arg  = Get-FirstArgument -Text $Text -Open $open
        if ($null -eq $arg) { continue }

        $id = Resolve-RuleId -Argument $arg -Values $Values
        if ($null -eq $id)
        {
            $ids += ('expr ' + ($arg -replace '\s+', ' ').Trim())
            ++$Dynamic.Value
            continue
        }

        $ids += [string] $id
    }

    return $ids
}

function Get-RevisionText
{
    # The file as the given revision holds it, or $null when git cannot supply it.
    param([string] $File, [string] $Rev)

    $directory = Split-Path -Parent (Resolve-Path $File)
    Push-Location $directory
    try
    {
        $top = (& git rev-parse --show-toplevel 2>$null)
        if ($LASTEXITCODE -ne 0) { return $null }

        $relative = (Resolve-Path $File).Path.Substring($top.Replace('/', '\').Length).TrimStart('\', '/').Replace('\', '/')
        $text     = (& git show ('{0}:{1}' -f $Rev, $relative) 2>$null) -join "`n"
        if ($LASTEXITCODE -ne 0) { return $null }

        return $text
    }
    finally
    {
        Pop-Location
    }
}

if (-not (Test-Path $Path)) { Write-Host "RULEAUDIT: FAIL | no source at $Path"; exit 1 }

$values      = @{}
$constantSet = @()
foreach ($file in $Constants)
{
    if (-not (Test-Path $file)) { Write-Host "RULEAUDIT: FAIL | no constants at $file"; exit 1 }

    $text = [System.IO.File]::ReadAllText($file)
    foreach ($pair in (Get-ConstantValues -Text $text).GetEnumerator())
    {
        $values[$pair.Key] = $pair.Value
    }

    $constantSet += $text
}

$currentText = [System.IO.File]::ReadAllText($Path)
foreach ($text in @($constantSet + $currentText)) { Add-Aliases -Values $values -Text $text }

$dynamic = 0
$current = @(Get-ReportedIds -Text $currentText -Values $values -Dynamic ([ref] $dynamic))

if ($List)
{
    Write-Host (($current | ForEach-Object { $_ }) -join ' ')
    Write-Host ('RULEAUDIT: LISTED | sites={0}' -f $current.Count)
    exit 0
}

$baseText = Get-RevisionText -File $Path -Rev $Revision
if ($null -eq $baseText)
{
    Write-Host ('RULEAUDIT: FAIL | {0} does not hold this file' -f $Revision)
    exit 1
}

$baseDynamic = 0
$base = @(Get-ReportedIds -Text $baseText -Values $values -Dynamic ([ref] $baseDynamic))

if ($base.Count -ne $current.Count)
{
    Write-Host ('RULEAUDIT: FAIL | {0} reporting calls now, {1} at {2}' -f $current.Count, $base.Count, $Revision)
    exit 1
}

for ($i = 0; $i -lt $base.Count; ++$i)
{
    if ($base[$i] -ne $current[$i])
    {
        Write-Host ('DIFFERS: call {0} reports {1}, it reported {2} at {3}' -f ($i + 1), $current[$i], $base[$i], $Revision)
        Write-Host ('RULEAUDIT: FAIL | the reported ids changed')
        exit 1
    }
}

Write-Host ('RULEAUDIT: PASS | sites={0} identical to {1}' -f $current.Count, $Revision)
exit 0

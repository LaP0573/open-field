# open-field helper for Windows: the file work of the install, on the OP-1 Field's disk.
#
# On Windows a web page cannot reach the unit's files, so this does it, with the shell Windows itself uses for the
# unit (the unit shows as a portable device named OP-1 while its bootloader's drive mode is open). It works with the
# install page through one folder on this computer, the working folder the owner picked on the page (any folder):
#   1. it finds the unit, copies your own firmware file off (upgrade/firmware/blackfin/bin/os.ldr), takes the log
#      files off into the folder, lists the disk, and waits;
#   2. the page builds the new firmware file, works out whether the write lands, and writes a plan into the folder;
#   3. this puts the new file on the unit as os.ldr (and a pad file and pack files when the plan says so), lists the
#      disk again, holds the listing against the plan row for row, and reads every file it copied back against its
#      SHA-256. Then it says READY TO WRITE.
# It decides nothing about the write's placement: the page does, and this only compares.
#
# What it changes on the unit, and nothing else: it takes the files in "logs" off, and the pad files of earlier
# installs that bear its own name (open-field-pad.bin, open-field-pad-2.bin, ...), and the files in user/pkin, the
# waiting folder where the install page leaves pack files over USB MIDI (its note .apply among them), and it writes
# os.ldr at the top level, the pad file the plan names, and the pack files the plan names (base.pack, from the folder's
# "packs" sub folder) into user/pk (the folder made if it is not there). It never deletes a folder, and it writes nothing before it has listed
# the disk again and found it as the plan was made from.
#
# Run it from a PowerShell window (the install page shows the command), or as a file:
#   powershell -NoProfile -ExecutionPolicy Bypass -File open-field-helper.ps1
# -Page, or $openFieldPage set before the script runs as the page's line does it, is the install page's mark: the page
# writes it into the folder it chose (page.json), and this checks it before anything else, so that the two never work
# in two different folders. Without a mark nothing is checked.
# -FolderName, or $openFieldFolder set the same way, is that folder's name. A browser tells a page a folder's name and
# never its path, so this looks for the folder: one of that name that holds the page's mark, in the current folder,
# the user folder, the Desktop, Documents and Downloads, each and one level below. With exactly one such folder it
# works there; with none or several it asks once, with a folder dialog, and checks the mark of the folder picked.
# -Folder names the folder outright (no search). -NoDialog stops where the dialog would open (for the tests).
# -Source names a folder of this computer that stands for the unit's disk, read and written through the same shell
# calls: the check of this script without a unit.
param(
    [string] $Folder = '',
    [string] $FolderName = '',
    [string] $Source = '',
    [int] $WaitMinutes = 60,
    [int] $FindSeconds = 180,
    [string] $Page = '',
    [switch] $NoDialog
)

# The mark the install page wrote into a folder (page.json), or $null.
function Read-OpenFieldMark([string] $Dir) {
    $markPath = Join-Path $Dir 'page.json'
    if (-not (Test-Path -LiteralPath $markPath -PathType Leaf)) { return $null }
    try { return [string] ((Get-Content -Raw -LiteralPath $markPath -Encoding UTF8 | ConvertFrom-Json).id) } catch { return $null }
}

# The folders that may be the one the page picked: of the name given (any letters' case) and holding the mark given,
# among the places an owner makes a folder in, each and one level below. Folders that cannot be read are passed over.
function Find-OpenFieldFolder([string] $Name, [string] $Mark) {
    # a full path with no backslash at its end, but for a drive's own top (C:\)
    $tidy = { param([string] $p) $f = [System.IO.Path]::GetFullPath($p); if ($f.Length -gt 3) { $f = $f.TrimEnd('\') }; $f }
    $roots = New-Object System.Collections.Generic.List[string]
    $here = $null
    try { $here = (Get-Location -PSProvider FileSystem).Path } catch { $here = $null }
    $downloads = if ($env:USERPROFILE) { Join-Path $env:USERPROFILE 'Downloads' } else { $null }
    foreach ($r in @($here, $env:USERPROFILE, [Environment]::GetFolderPath('Desktop'), [Environment]::GetFolderPath('MyDocuments'), $downloads)) {
        if ($r -and (Test-Path -LiteralPath $r -PathType Container)) {
            $full = & $tidy $r
            if (-not ($roots | Where-Object { $_ -ieq $full })) { $roots.Add($full) }
        }
    }
    $found = New-Object System.Collections.Generic.List[string]
    foreach ($root in $roots) {
        $places = @($root)
        try { $places += @([System.IO.Directory]::GetDirectories($root)) } catch { }
        foreach ($p in $places) {
            $full = & $tidy $p
            if ($Name -and -not ((Split-Path -Leaf $full) -ieq $Name)) { continue }
            if ($Mark -and (Read-OpenFieldMark $full) -ne $Mark) { continue }
            if (-not ($found | Where-Object { $_ -ieq $full })) { $found.Add($full) }
        }
    }
    return ,$found.ToArray()
}

# The owner picks the folder in a dialog: '' when the dialog is closed with no folder.
function Select-OpenFieldFolder([string] $Name) {
    $title = 'Pick the working folder you chose on the install page' + $(if ($Name) { ' (' + $Name + ')' } else { '' })
    try {
        Add-Type -AssemblyName System.Windows.Forms
        $dialog = New-Object System.Windows.Forms.FolderBrowserDialog
        $dialog.Description = $title
        $dialog.ShowNewFolderButton = $false
        $owner = New-Object System.Windows.Forms.Form
        $owner.TopMost = $true
        try {
            if ($dialog.ShowDialog($owner) -eq [System.Windows.Forms.DialogResult]::OK) { return [string] $dialog.SelectedPath }
            return ''
        } finally { $owner.Dispose(); $dialog.Dispose() }
    } catch {
        # a shell where that dialog cannot open (not on a single-threaded apartment): the shell's own folder dialog
        $picked = (New-Object -ComObject Shell.Application).BrowseForFolder(0, $title, 0x41, 0)
        if ($picked) { return [string] $picked.Self.Path }
        return ''
    }
}

function Start-OpenFieldHelper {
    param([string] $Folder, [string] $FolderName, [string] $Source, [int] $WaitMinutes, [int] $FindSeconds, [string] $Page, [bool] $NoDialog)
    $ErrorActionPreference = 'Stop'
    $HelperVersion = 1
    # The original OS 1.7.3 file, as the builder's manifest names it: the only file kept under the name "yours".
    $OriginalSha = 'd134eb1ac7ad88e8370d988329cd5d7419f016005b1d269034f9cf7166fad0ec'
    $Names = @{ stock = 'os-1.7.3-yours.ldr'; image = 'open-field-os.ldr'; helper = 'helper.json'; before = 'listing-before.json';
                after = 'listing-after.json'; plan = 'plan.json'; packs = 'packs' }
    if ($WaitMinutes -le 0) { $WaitMinutes = 60 }
    if ($FindSeconds -le 0) { $FindSeconds = 180 }
    # The folder the page picked: looked for by its name and the page's mark, or picked in a dialog.
    if (-not $Folder) {
        $hits = @()
        if ($FolderName -or $Page) { $hits = Find-OpenFieldFolder $FolderName $Page }
        if ($hits.Count -eq 1) { $Folder = $hits[0] }
        else {
            $what = if ($FolderName) { 'the working folder "' + $FolderName + '"' } else { 'the working folder' }
            $why = if ($hits.Count -gt 1) { 'Found ' + $hits.Count + ' folders that could be ' + $what + '.' } else { 'Could not find ' + $what + ' by itself.' }
            if ($NoDialog) {
                Write-Host ('STOPPED: ' + $why + ' Nothing was done.') -ForegroundColor Red
                return
            }
            Write-Host ($why + ' Pick it in the window that opens.') -ForegroundColor Yellow
            $Folder = Select-OpenFieldFolder $FolderName
            if (-not $Folder) {
                Write-Host 'STOPPED: no folder was picked. Nothing was done.' -ForegroundColor Red
                return
            }
        }
    }
    # The page's mark, before anything is made or written: the folder the page chose must be this one.
    if ($Page -and (Read-OpenFieldMark $Folder) -ne $Page) {
        Write-Host ('STOPPED: the install page did not choose this folder: ' + $Folder) -ForegroundColor Red
        Write-Host 'On the install page, step 3, pick your working folder again,' -ForegroundColor Red
        Write-Host 'then copy the line of step 4 again and run it. Nothing was done.' -ForegroundColor Red
        return
    }
    New-Item -ItemType Directory -Force -Path $Folder | Out-Null
    $Folder = (Resolve-Path -LiteralPath $Folder).Path
    $run = Get-Date -Format 'yyyy-MM-dd_HH-mm-ss'
    $logFile = Join-Path $Folder ('helper-' + $run + '.log')
    $state = [ordered]@{ helper = $HelperVersion; run = $run; page = $(if ($Page) { $Page } else { $null }); state = 'starting'; time = (Get-Date).ToString('s'); folder = $Folder;
                         unit = $null; stock = $null; logs = $null; oldPads = $null; waiting = $null; listing = $null; checks = @(); lines = @(); message = $null }
    $shell = New-Object -ComObject Shell.Application
    $utf8 = New-Object System.Text.UTF8Encoding $false

    # Written beside and put in the place of the old file in one step, so that the page, which reads the folder every
    # second, never reads half a file and never finds none (a move over a file takes the old one away first, and a
    # read in between found nothing: one cause of the step that jumped on the page, 2026-11-20). The page may hold the
    # file for a moment while it reads it, so the swap is tried again a few times.
    function Save-Text([string] $path, [string] $text) {
        $part = $path + '.part'
        [System.IO.File]::WriteAllText($part, $text, $utf8)
        for ($try = 1; $try -le 20; $try++) {
            try {
                if (Test-Path -LiteralPath $path) { [System.IO.File]::Replace($part, $path, [NullString]::Value) }
                else { [System.IO.File]::Move($part, $path) }
                return
            } catch { Start-Sleep -Milliseconds 50 }
        }
        Move-Item -LiteralPath $part -Destination $path -Force
    }
    function Save-State([string] $now) {
        $state.state = $now
        $state.time = (Get-Date).ToString('s')
        Save-Text (Join-Path $Folder $Names.helper) (ConvertTo-Json -InputObject $state -Depth 8)
    }
    function Say([string] $text, [string] $color = '') {
        $line = (Get-Date -Format 'HH:mm:ss') + '  ' + $text
        if ($color) { Write-Host $line -ForegroundColor $color } else { Write-Host $line }
        Add-Content -LiteralPath $logFile -Value $line -Encoding UTF8
        $state.lines = @($state.lines) + @($text)
    }
    function Stop-Helper([string] $why) {
        Say ('STOPPED: ' + $why) 'Red'
        Say 'Nothing more is done. The unit is as this line finds it; read the lines above. Run the command again to start over.' 'Red'
        $state.message = $why
        Save-State 'stopped'
        throw 'open-field-helper-stopped'
    }
    function Get-Sha([string] $path) { (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLower() }
    function Get-Name($item) {
        $n = $null
        try { $n = $item.ExtendedProperty('System.FileName') } catch { $n = $null }
        if (-not $n) { $n = $item.Name }
        return [string] $n
    }
    function Get-Size($item) {
        $s = $null
        try { $s = $item.ExtendedProperty('System.Size') } catch { $s = $null }
        if ($null -eq $s) { return [int64] -1 }
        return [int64] $s
    }
    function Find-Child($folder, [string] $name) {
        foreach ($i in @($folder.Items())) { if ((Get-Name $i) -eq $name) { return $i } }
        return $null
    }
    # The unit's top folder: the portable device named OP-1 and its first storage, or the -Source folder.
    function Get-Top([int] $seconds) {
        if ($Source) {
            $t = $shell.NameSpace($Source)
            if (-not $t) { Stop-Helper ('no such folder: ' + $Source) }
            return $t
        }
        $deadline = (Get-Date).AddSeconds($seconds)
        $told = $false
        while ($true) {
            $units = @($shell.NameSpace(17).Items() | Where-Object { $_.IsFolder -and $_.Name -eq 'OP-1' })
            if ($units.Count -gt 1) { Stop-Helper 'more than one OP-1 is connected: connect only the unit you are installing on' }
            if ($units.Count -eq 1) {
                $storage = @($units[0].GetFolder.Items()) | Select-Object -First 1
                if ($storage) { return $storage.GetFolder }
            }
            if ((Get-Date) -gt $deadline) { Stop-Helper 'the unit did not show on this computer. Hold COM while switching it on, press 3, then 2, with the cable in; then run the command again' }
            if (-not $told) { Say 'Waiting for the unit: on the unit, in the bootloader menu, press 3, then 2. The cable must be in.' 'Yellow'; $told = $true }
            Start-Sleep -Seconds 2
        }
    }
    function Resolve-Folder($top, [string] $path) {
        $f = $top
        foreach ($part in @($path.Split('/') | Where-Object { $_ -ne '' })) {
            $next = Find-Child $f $part
            if (-not $next -or -not $next.IsFolder) { return $null }
            $f = $next.GetFolder
        }
        return $f
    }
    function Get-Listing($top) {
        $rows = New-Object System.Collections.Generic.List[object]
        $walk = {
            param($folder, [string] $prefix)
            foreach ($item in @($folder.Items())) {
                $path = $prefix + (Get-Name $item)
                if ($item.IsFolder) {
                    $rows.Add([pscustomobject]@{ path = $path; dir = $true })
                    & $walk $item.GetFolder ($path + '/')
                } else {
                    $rows.Add([pscustomobject]@{ path = $path; size = (Get-Size $item); dir = $false })
                }
            }
        }
        & $walk $top ''
        return ,$rows.ToArray()
    }
    function Get-Keys($rows) {
        $set = New-Object 'System.Collections.Generic.HashSet[string]'
        foreach ($r in $rows) {
            if ($r.dir -eq $true) { [void] $set.Add('D ' + $r.path) } else { [void] $set.Add('F ' + $r.path + ' ' + [int64] $r.size) }
        }
        return ,$set
    }
    function Compare-Rows($want, $have) {
        $a = Get-Keys $want
        $b = Get-Keys $have
        $missing = @($a | Where-Object { -not $b.Contains($_) } | Sort-Object)
        $extra = @($b | Where-Object { -not $a.Contains($_) } | Sort-Object)
        return [pscustomobject]@{ Same = ($missing.Count -eq 0 -and $extra.Count -eq 0); Missing = $missing; Extra = $extra }
    }
    function Save-Listing($rows, [string] $name) {
        Save-Text (Join-Path $Folder $name) (ConvertTo-Json -InputObject @($rows) -Compress -Depth 3)
    }
    # A copy off the unit into a folder of this computer, waited for until the file has the size the unit gives.
    function Copy-Off($item, [string] $destDir, [int] $seconds = 300) {
        New-Item -ItemType Directory -Force -Path $destDir | Out-Null
        $target = Join-Path $destDir (Get-Name $item)
        if (Test-Path -LiteralPath $target) { Remove-Item -LiteralPath $target -Force }
        $want = Get-Size $item
        $shell.NameSpace($destDir).CopyHere($item, 4 + 16 + 1024)
        $deadline = (Get-Date).AddSeconds($seconds)
        while ((Get-Date) -lt $deadline) {
            if (Test-Path -LiteralPath $target) {
                if ($want -lt 0 -or (Get-Item -LiteralPath $target).Length -eq $want) { Start-Sleep -Milliseconds 300; return $target }
            }
            Start-Sleep -Milliseconds 200
        }
        return $null
    }
    # A file of this computer onto the unit, waited for until the unit lists it with its size.
    function Copy-On([string] $localPath, $top, [string] $folderPath, [int64] $bytes, [int] $seconds = 300) {
        $name = Split-Path -Leaf $localPath
        $dest = if ($folderPath) { Resolve-Folder $top $folderPath } else { $top }
        if (-not $dest) { Stop-Helper ('the unit has no folder ' + $folderPath) }
        $dest.CopyHere($localPath, 4 + 16 + 1024)
        $deadline = (Get-Date).AddSeconds($seconds)
        while ((Get-Date) -lt $deadline) {
            Start-Sleep -Milliseconds 500
            $d = if ($folderPath) { Resolve-Folder $top $folderPath } else { $top }
            if ($d) {
                $it = Find-Child $d $name
                if ($it -and -not $it.IsFolder -and (Get-Size $it) -eq $bytes) { return $it }
            }
        }
        Stop-Helper ('the unit did not show ' + (($folderPath, $name) -join '/').TrimStart('/') + ' with its ' + $bytes + ' bytes in ' + $seconds + ' seconds')
    }
    # A file the helper put on the unit, read back and held to its SHA-256; read again a few times, since the unit may
    # list a file with its size before the last of it has arrived.
    function Test-ReadBack($top, [string] $path, [string] $sha, [string] $dir) {
        $parent = if ($path.Contains('/')) { $path.Substring(0, $path.LastIndexOf('/')) } else { '' }
        $leaf = $path.Substring($path.LastIndexOf('/') + 1)
        for ($try = 1; $try -le 5; $try++) {
            $f = if ($parent) { Resolve-Folder $top $parent } else { $top }
            $it = if ($f) { Find-Child $f $leaf } else { $null }
            if ($it) {
                $copy = Copy-Off $it $dir 300
                if ($copy) {
                    $got = Get-Sha $copy
                    Remove-Item -LiteralPath $copy -Force
                    if ($got -eq $sha) { return $true }
                    Say ('  ' + $path + ' read back with another SHA-256 (' + $got.Substring(0, 12) + '...), try ' + $try + ' of 5') 'Yellow'
                }
            }
            Start-Sleep -Seconds 2
        }
        return $false
    }

    try {
        Say ('open-field helper ' + $HelperVersion + ', run ' + $run + '. The folder: ' + $Folder) 'Cyan'
        foreach ($old in @($Names.plan, $Names.after)) {
            $p = Join-Path $Folder $old
            if (Test-Path -LiteralPath $p) { Remove-Item -LiteralPath $p -Force }
        }
        Save-State 'starting'

        # ---- 1. the unit ----
        $top = Get-Top $FindSeconds
        $state.unit = if ($Source) { 'a folder standing for the unit: ' + $Source } else { 'OP-1' }
        Say ('Found the unit''s drive (' + $state.unit + ').') 'Green'

        # ---- 2. your own firmware file, copied off ----
        $bin = Resolve-Folder $top 'upgrade/firmware/blackfin/bin'
        $osItem = if ($bin) { Find-Child $bin 'os.ldr' } else { $null }
        if (-not $osItem -or $osItem.IsFolder) {
            Say 'The unit has no upgrade/firmware/blackfin/bin/os.ldr. The page will ask for the firmware file you kept.' 'Yellow'
        } else {
            $incoming = Join-Path $Folder ('incoming-' + $run)
            $copy = Copy-Off $osItem $incoming 300
            if (-not $copy) { Stop-Helper 'the firmware file did not come off the unit' }
            $sha = Get-Sha $copy
            $bytes = (Get-Item -LiteralPath $copy).Length
            $original = ($sha -eq $OriginalSha)
            # The original file is kept as os-1.7.3-yours.ldr, once; another file by a name that says what it is.
            $name = if ($original) { $Names.stock } else { 'os-from-upgrade-folder-' + $run + '.ldr' }
            $kept = Join-Path $Folder $name
            if ((Test-Path -LiteralPath $kept) -and (Get-Sha $kept) -eq $sha) { Remove-Item -LiteralPath $copy -Force }
            else {
                if (Test-Path -LiteralPath $kept) { $name = 'os-1.7.3-yours-' + $run + '.ldr'; $kept = Join-Path $Folder $name }
                Move-Item -LiteralPath $copy -Destination $kept
            }
            Remove-Item -LiteralPath $incoming -Recurse -Force
            $state.stock = [ordered]@{ file = $name; bytes = $bytes; sha256 = $sha; original = $original; from = 'upgrade/firmware/blackfin/bin/os.ldr' }
            if ($original) { Say ('Your own firmware file is copied off: ' + $name + ' (the original OS 1.7.3 file, ' + $bytes + ' bytes).') 'Green' }
            else { Say ('The unit''s upgrade folder holds another file than the original OS 1.7.3 one (' + $bytes + ' bytes); it is copied off as ' + $name + '. The page takes the original from this folder (' + $Names.stock + ') if it is there, or asks for it.') 'Yellow' }
        }

        # ---- 3. the log files, taken off ----
        $logs = Find-Child $top 'logs'
        $moved = 0; $left = 0; $logDir = $null
        if ($logs -and $logs.IsFolder) {
            $files = @($logs.GetFolder.Items() | Where-Object { -not $_.IsFolder })
            if ($files.Count -gt 0) {
                $logDir = Join-Path $Folder ('logs-' + $run)
                New-Item -ItemType Directory -Force -Path $logDir | Out-Null
                foreach ($item in $files) {
                    $name = Get-Name $item
                    $want = Get-Size $item
                    $shell.NameSpace($logDir).MoveHere($item, 4 + 16 + 1024)
                    $target = Join-Path $logDir $name
                    $deadline = (Get-Date).AddSeconds(120)
                    while ((Get-Date) -lt $deadline -and -not ((Test-Path -LiteralPath $target) -and ($want -lt 0 -or (Get-Item -LiteralPath $target).Length -eq $want))) { Start-Sleep -Milliseconds 200 }
                }
                Start-Sleep -Seconds 1
                $logsNow = Resolve-Folder $top 'logs'
                $left = if ($logsNow) { @($logsNow.Items() | Where-Object { -not $_.IsFolder }).Count } else { 0 }
                $moved = @(Get-ChildItem -LiteralPath $logDir -File).Count
            }
        }
        $state.logs = [ordered]@{ folder = $(if ($logDir) { Split-Path -Leaf $logDir } else { $null }); copied = $moved; left_on_unit = $left }
        if ($moved -eq 0 -and $left -eq 0) { Say 'The unit''s logs folder holds no file.' }
        elseif ($left -eq 0) { Say ('The ' + $moved + ' log file(s) are taken off the unit into ' + (Split-Path -Leaf $logDir) + '.') 'Green' }
        else { Say ($left + ' log file(s) stay on the unit (' + $moved + ' copied). That is no fault: the plan counts the disk as it is.') 'Yellow' }

        # ---- 3b. the pad files of earlier installs, taken off ----
        # A pad file serves one write only. The helper's own (open-field-pad.bin, open-field-pad-2.bin, ...) are taken off
        # before the listing, as the logs are, so that the plan counts the disk without them and at most one is ever on the
        # unit (the owner's word, 2026-11-18). A file of any other name is left where it is. Their bytes are of no use
        # (only a pad file's size ever counted), so the copies the move leaves on this computer are deleted.
        $padName = '^open-field-pad(-[0-9]+)?\.bin$'
        $oldPads = @(@($top.Items()) | Where-Object { -not $_.IsFolder -and (Get-Name $_) -match $padName })
        $padsLeft = 0
        if ($oldPads.Count -gt 0) {
            $padDir = Join-Path $Folder ('old-pads-' + $run)
            New-Item -ItemType Directory -Force -Path $padDir | Out-Null
            foreach ($item in $oldPads) {
                $name = Get-Name $item
                $want = Get-Size $item
                $shell.NameSpace($padDir).MoveHere($item, 4 + 16 + 1024)
                $target = Join-Path $padDir $name
                $deadline = (Get-Date).AddSeconds(300)
                while ((Get-Date) -lt $deadline -and -not ((Test-Path -LiteralPath $target) -and ($want -lt 0 -or (Get-Item -LiteralPath $target).Length -eq $want))) { Start-Sleep -Milliseconds 200 }
            }
            Start-Sleep -Seconds 1
            $padsLeft = @(@($top.Items()) | Where-Object { -not $_.IsFolder -and (Get-Name $_) -match $padName }).Count
            Remove-Item -LiteralPath $padDir -Recurse -Force
        }
        $state.oldPads = [ordered]@{ found = $oldPads.Count; taken_off = ($oldPads.Count - $padsLeft); left_on_unit = $padsLeft }
        if ($oldPads.Count -gt 0) {
            if ($padsLeft -eq 0) { Say ('The ' + $oldPads.Count + ' pad file(s) of earlier installs are taken off the unit.') 'Green' }
            else { Say ($padsLeft + ' pad file(s) of earlier installs stay on the unit. That is no fault: the plan counts the disk as it is.') 'Yellow' }
        }

        # ---- 3c. the waiting folder of the packs, emptied ----
        # The install page sends pack files over USB MIDI into user/pkin, where they wait for the owner's T1 on the unit,
        # and the unit keeps a note there (.apply) while it changes its packs. An install puts a base.pack on the unit,
        # and a note left behind must never meet it: the very same base.pack file cannot be told from the one that wrote
        # the note. The plan is made after the listing, so whether this run writes base.pack is not known yet; every
        # run takes the files of user/pkin off, the note among them, before the listing, as it does the logs, so that
        # the plan counts the disk without them (the way back loses nothing by it: the files that wait there are lost
        # at the unit's next start anyway, and the original firmware never reads them). The folder itself stays.
        $pkin = Resolve-Folder $top 'user/pkin'
        $waitingFound = 0; $waitingLeft = 0
        if ($pkin) {
            $waitingFiles = @(@($pkin.Items()) | Where-Object { -not $_.IsFolder })
            $waitingFound = $waitingFiles.Count
            if ($waitingFound -gt 0) {
                $waitDir = Join-Path $Folder ('old-pkin-' + $run)
                New-Item -ItemType Directory -Force -Path $waitDir | Out-Null
                foreach ($item in $waitingFiles) {
                    $name = Get-Name $item
                    $want = Get-Size $item
                    $shell.NameSpace($waitDir).MoveHere($item, 4 + 16 + 1024)
                    $target = Join-Path $waitDir $name
                    $deadline = (Get-Date).AddSeconds(120)
                    while ((Get-Date) -lt $deadline -and -not ((Test-Path -LiteralPath $target) -and ($want -lt 0 -or (Get-Item -LiteralPath $target).Length -eq $want))) { Start-Sleep -Milliseconds 200 }
                }
                Start-Sleep -Seconds 1
                $pkinNow = Resolve-Folder $top 'user/pkin'
                $waitingLeft = if ($pkinNow) { @(@($pkinNow.Items()) | Where-Object { -not $_.IsFolder }).Count } else { 0 }
                Remove-Item -LiteralPath $waitDir -Recurse -Force
            }
        }
        $state.waiting = [ordered]@{ found = $waitingFound; taken_off = ($waitingFound - $waitingLeft); left_on_unit = $waitingLeft }
        if ($waitingFound -gt 0) {
            if ($waitingLeft -eq 0) { Say ('The ' + $waitingFound + ' file(s) waiting in user/pkin are taken off the unit.') 'Green' }
            else { Stop-Helper ($waitingLeft + ' file(s) stay in user/pkin on the unit and would meet the new base.pack. Delete them in the unit''s drive, then run the command again') }
        }

        # ---- 4. the disk, listed ----
        $rows = Get-Listing $top
        Save-Listing $rows $Names.before
        $nFiles = @($rows | Where-Object { -not $_.dir }).Count
        $nFolders = @($rows | Where-Object { $_.dir }).Count
        $state.listing = [ordered]@{ file = $Names.before; files = $nFiles; folders = $nFolders; sha256 = (Get-Sha (Join-Path $Folder $Names.before)) }
        Say ('The disk is listed: ' + $nFiles + ' files, ' + $nFolders + ' folders.') 'Green'
        Save-State 'listed'

        # ---- 5. the page's plan ----
        Say 'Now go back to the install page. This window waits for it; leave it open.' 'Cyan'
        $planPath = Join-Path $Folder $Names.plan
        $deadline = (Get-Date).AddMinutes($WaitMinutes)
        $plan = $null
        $nextNote = (Get-Date).AddMinutes(2)
        while (-not $plan) {
            if (Test-Path -LiteralPath $planPath) {
                try { $p = Get-Content -LiteralPath $planPath -Raw -Encoding UTF8 | ConvertFrom-Json } catch { $p = $null }
                if ($p -and $p.run -eq $run) { $plan = $p; break }
            }
            if ((Get-Date) -gt $deadline) { Stop-Helper ('no plan came from the page in ' + $WaitMinutes + ' minutes') }
            if ((Get-Date) -gt $nextNote) { Say '  still waiting for the page...'; $nextNote = (Get-Date).AddMinutes(2) }
            Start-Sleep -Seconds 1
        }
        Say 'The page''s plan is here.' 'Green'
        if ($plan.plan -ne 1) { Stop-Helper ('the plan is of a kind this helper does not know (' + $plan.plan + ')') }
        if (-not $plan.before -or $plan.before.sha256 -ne $state.listing.sha256) { Stop-Helper 'the plan was not made from this run''s listing' }
        $imagePath = Join-Path $Folder $plan.image.file
        if (-not (Test-Path -LiteralPath $imagePath) -or (Get-Sha $imagePath) -ne $plan.image.sha256) { Stop-Helper ('the new firmware file ' + $plan.image.file + ' is not in the folder as the plan says') }
        if ($plan.pad) {
            $padPath = Join-Path $Folder $plan.pad.file
            if (-not (Test-Path -LiteralPath $padPath) -or (Get-Sha $padPath) -ne $plan.pad.sha256) { Stop-Helper ('the pad file ' + $plan.pad.file + ' is not in the folder as the plan says') }
        }
        foreach ($pk in @($plan.packs)) {
            if (-not $pk) { continue }
            $pp = Join-Path $Folder ($pk.file -replace '/', '\')
            if (-not (Test-Path -LiteralPath $pp) -or (Get-Sha $pp) -ne $pk.sha256) { Stop-Helper ('the pack file ' + $pk.file + ' is not in the folder as the plan says') }
        }

        # ---- 6. the disk as the plan was made from it? ----
        $top = Get-Top 10
        $now = Get-Listing $top
        $same = Compare-Rows $rows $now
        if (-not $same.Same) {
            foreach ($k in @($same.Missing | Select-Object -First 10)) { Say ('  gone since the listing: ' + $k) 'Red' }
            foreach ($k in @($same.Extra | Select-Object -First 10)) { Say ('  new since the listing: ' + $k) 'Red' }
            Stop-Helper 'the unit''s disk changed since it was listed, so the plan no longer holds'
        }
        Say 'The disk is as it was listed.' 'Green'

        # ---- 7. the files onto the unit ----
        Save-State 'putting'
        $stage = Join-Path $Folder ('to-unit-' + $run)
        New-Item -ItemType Directory -Force -Path $stage | Out-Null
        $staged = Join-Path $stage 'os.ldr'
        Copy-Item -LiteralPath $imagePath -Destination $staged -Force
        if ((Get-Sha $staged) -ne $plan.image.sha256) { Stop-Helper 'the copy of the new firmware file on this computer is not the plan''s' }
        Copy-On $staged $top '' ([int64] $plan.image.bytes) 600 | Out-Null
        Say ('os.ldr is on the unit (' + $plan.image.bytes + ' bytes).') 'Green'
        if ($plan.pad) {
            Copy-On (Join-Path $Folder $plan.pad.file) $top '' ([int64] $plan.pad.bytes) 600 | Out-Null
            Say ('The pad file ' + $plan.pad.name + ' is on the unit (' + $plan.pad.bytes + ' bytes).') 'Green'
        }
        if ($plan.makeFolder) {
            $parentPath = $plan.makeFolder.Substring(0, $plan.makeFolder.LastIndexOf('/'))
            $leaf = $plan.makeFolder.Substring($plan.makeFolder.LastIndexOf('/') + 1)
            $parent = Resolve-Folder $top $parentPath
            if (-not $parent) { Stop-Helper ('the unit has no folder ' + $parentPath) }
            $parent.NewFolder($leaf)
            $deadline = (Get-Date).AddSeconds(60)
            while (-not (Resolve-Folder $top $plan.makeFolder)) {
                if ((Get-Date) -gt $deadline) { Stop-Helper ('the folder ' + $plan.makeFolder + ' did not appear on the unit') }
                Start-Sleep -Milliseconds 500
            }
            Say ('The folder ' + $plan.makeFolder + ' is made on the unit.') 'Green'
        }
        foreach ($pk in @($plan.packs)) {
            if (-not $pk) { continue }
            $folderPath = $pk.path.Substring(0, $pk.path.LastIndexOf('/'))
            Copy-On (Join-Path $Folder ($pk.file -replace '/', '\')) $top $folderPath ([int64] $pk.bytes) 600 | Out-Null
            Say ('The pack file ' + $pk.name + ' is on the unit.') 'Green'
        }

        # ---- 8. the disk listed again, held against the plan ----
        $afterRows = Get-Listing $top
        Save-Listing $afterRows $Names.after
        $diff = Compare-Rows @($plan.expected) $afterRows
        if (-not $diff.Same) {
            foreach ($k in @($diff.Missing | Select-Object -First 10)) { Say ('  planned and not on the unit: ' + $k) 'Red' }
            foreach ($k in @($diff.Extra | Select-Object -First 10)) { Say ('  on the unit and not planned: ' + $k) 'Red' }
            Stop-Helper 'the disk is not as the plan says it must be for the write'
        }
        $state.checks = @($state.checks) + @('the listing is the plan''s, row for row (' + @($plan.expected).Count + ' rows)')
        Say ('The disk''s new listing is the plan''s, row for row.') 'Green'

        # ---- 9. every copied file read back ----
        $back = Join-Path $Folder ('read-back-' + $run)
        $wanted = @(@{ path = 'os.ldr'; sha = $plan.image.sha256 })
        if ($plan.pad) { $wanted += @{ path = $plan.pad.name; sha = $plan.pad.sha256 } }
        foreach ($pk in @($plan.packs)) { if ($pk) { $wanted += @{ path = $pk.path; sha = $pk.sha256 } } }
        foreach ($w in $wanted) {
            if (-not (Test-ReadBack $top $w.path $w.sha $back)) { Stop-Helper ($w.path + ' does not read back from the unit with the SHA-256 of the file sent') }
            $state.checks = @($state.checks) + @($w.path + ' read back: ' + $w.sha)
            Say ($w.path + ' read back from the unit: the same bytes.') 'Green'
        }
        foreach ($d in @($stage, $back)) { if (Test-Path -LiteralPath $d) { Remove-Item -LiteralPath $d -Recurse -Force } }

        $state.message = 'ready to write'
        Save-State 'ready'
        Write-Host ''
        Say 'READY TO WRITE.' 'Green'
        Say 'Now restart your OP-1 in bootloader mode: turn it off, then turn it on while holding COM.' 'Green'
        Say 'Do NOT restart it in normal mode until the install is finished, otherwise you will have to start again.' 'Green'
        Say 'Step 7 of the install page has the rest.' 'Green'
    } catch {
        if ("$_" -ne 'open-field-helper-stopped') {
            try { Say ('STOPPED by an error: ' + $_) 'Red'; $state.message = "$_"; Save-State 'stopped' } catch { Write-Host ('STOPPED by an error: ' + $_) }
        }
    }
}

# The page's line sets $openFieldPage and $openFieldFolder before it hands this text to the shell, where a parameter
# cannot be passed.
if (-not $Page -and (Get-Variable -Name openFieldPage -ErrorAction SilentlyContinue)) { $Page = [string] $openFieldPage }
if (-not $FolderName -and (Get-Variable -Name openFieldFolder -ErrorAction SilentlyContinue)) { $FolderName = [string] $openFieldFolder }
Start-OpenFieldHelper -Folder $Folder -FolderName $FolderName -Source $Source -WaitMinutes $WaitMinutes -FindSeconds $FindSeconds -Page $Page -NoDialog $NoDialog.IsPresent

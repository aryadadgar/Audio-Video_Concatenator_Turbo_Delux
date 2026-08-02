Imports System
Imports System.IO
Imports System.Linq
Imports System.Threading
Imports System.Threading.Tasks
Imports System.Windows.Forms
Imports System.Drawing
Imports System.Diagnostics
Imports Windows.Storage
Imports Windows.Media.Editing
Imports Windows.Media.Transcoding
Imports Windows.Media.MediaProperties

Public Class Form1
    Private lstFiles As ListBox
    Private btnMerge As Button
    Private btnClear As Button
    Private pbProgress As ProgressBar
    Private txtLog As TextBox

    ' --- THE TRUE ENTRY POINT FOR NET 11 ---
    <STAThread>
    Public Shared Sub Main()
        Application.SetHighDpiMode(HighDpiMode.SystemAware)
        Application.EnableVisualStyles()
        Application.SetCompatibleTextRenderingDefault(False)
        Application.Run(New Form1())
    End Sub

    Public Sub New()
        InitializeComponent()
        Me.Text = "Turbo Media Merger - CPU Stable Build"
        Me.Size = New Size(620, 520)
        Me.AllowDrop = True
        Me.StartPosition = FormStartPosition.CenterScreen

        lstFiles = New ListBox() With {.Location = New Point(15, 15), .Size = New Size(575, 160), .AllowDrop = True, .HorizontalScrollbar = True}
        btnMerge = New Button() With {.Location = New Point(15, 185), .Size = New Size(130, 35), .Text = "Start CPU Merge", .BackColor = Color.LightGreen}
        btnClear = New Button() With {.Location = New Point(155, 185), .Size = New Size(100, 35), .Text = "Clear List"}
        pbProgress = New ProgressBar() With {.Location = New Point(270, 185), .Size = New Size(320, 35)}
        txtLog = New TextBox() With {.Location = New Point(15, 230), .Size = New Size(575, 230), .Multiline = True, .ScrollBars = ScrollBars.Vertical, .ReadOnly = True, .BackColor = Color.Black, .ForeColor = Color.Lime, .Font = New Font("Consolas", 9)}

        Me.Controls.AddRange(New Control() {lstFiles, btnMerge, btnClear, pbProgress, txtLog})

        AddHandler Me.DragEnter, Sub(s, e) If e.Data.GetDataPresent(DataFormats.FileDrop) Then e.Effect = DragDropEffects.Copy
        AddHandler Me.DragDrop, AddressOf HandleDrop
        AddHandler btnClear.Click, Sub()
                                       lstFiles.Items.Clear()
                                       pbProgress.Value = 0
                                       txtLog.Clear()
                                   End Sub
        AddHandler btnMerge.Click, AddressOf RunMerge
    End Sub

    Private Sub HandleDrop(sender As Object, e As DragEventArgs)
        Dim files As String() = CType(e.Data.GetData(DataFormats.FileDrop), String())
        For Each f In files : lstFiles.Items.Add(f) : Next
    End Sub

    Private Sub RunMerge()
        Dim vFiles = lstFiles.Items.Cast(Of String)().Where(Function(f) f.ToLower().EndsWith(".mp4")).OrderBy(Function(f) f.Length).ThenBy(Function(f) f).ToList()
        Dim aFiles = lstFiles.Items.Cast(Of String)().Where(Function(f) Not f.ToLower().EndsWith(".mp4")).OrderBy(Function(f) f.Length).ThenBy(Function(f) f).ToList()

        If vFiles.Count = 0 OrElse aFiles.Count = 0 Then
            MessageBox.Show("Please add both MP4 and Audio files.")
            Return
        End If

        btnMerge.Enabled = False
        txtLog.Clear()
        txtLog.AppendText($"[INFO] Building timeline for {vFiles.Count} pairs..." & vbCrLf)

        ' Execute completely off the UI synchronization context thread to pin handles 
        Task.Run(Sub()
                     Try
                         Dim composition As New MediaComposition()
                         Dim currentTimeOffset = TimeSpan.Zero
                         Dim totalPairs = Math.Min(vFiles.Count, aFiles.Count)

                         For i As Integer = 0 To totalPairs - 1
                             ' Direct tracking block allocation prevents pipeline loss
                             Dim vidFile = StorageFile.GetFileFromPathAsync(vFiles(i)).AsTask().GetAwaiter().GetResult()
                             Dim audFile = StorageFile.GetFileFromPathAsync(aFiles(i)).AsTask().GetAwaiter().GetResult()

                             Dim clip = MediaClip.CreateFromFileAsync(vidFile).AsTask().GetAwaiter().GetResult()
                             Try : clip.Volume = 0.5 : Catch : End Try
                             composition.Clips.Add(clip)

                             Try
                                 Dim track = BackgroundAudioTrack.CreateFromFileAsync(audFile).AsTask().GetAwaiter().GetResult()
                                 track.Delay = currentTimeOffset
                                 track.Volume = 1.0
                                 If track.OriginalDuration > clip.OriginalDuration Then
                                     track.TrimTimeFromEnd = track.OriginalDuration - clip.OriginalDuration
                                 End If
                                 composition.BackgroundAudioTracks.Add(track)
                             Catch
                                 Me.BeginInvoke(Sub() txtLog.AppendText($"[WARN] Could not load audio for pair {i + 1}" & vbCrLf))
                             End Try

                             currentTimeOffset += clip.OriginalDuration
                             Dim currentName = Path.GetFileName(vFiles(i))
                             Me.BeginInvoke(Sub() txtLog.AppendText($"[QUEUED] {currentName}" & vbCrLf))
                         Next

                         ' Force CPU Fallback Profile explicitly
                         Dim profile = MediaEncodingProfile.CreateMp4(VideoEncodingQuality.HD1080p)
                         profile.Video.Bitrate = 3000000 
                         profile.Audio = AudioEncodingProperties.CreateAac(48000, 2, 192000)

                         Dim outDir = Path.GetDirectoryName(vFiles(0))
                         Dim sFolder = StorageFolder.GetFolderFromPathAsync(outDir).AsTask().GetAwaiter().GetResult()
                         Dim outFile = sFolder.CreateFileAsync("Final_Merged_3Mbps_CPU.mp4", CreationCollisionOption.ReplaceExisting).AsTask().GetAwaiter().GetResult()

                         Me.BeginInvoke(Sub() txtLog.AppendText("[RENDER] CPU Multiplex Rendering Processing Engaged..." & vbCrLf))

                         Dim sw As New Stopwatch()
                         sw.Start()
                         
                         Dim renderOp = composition.RenderToFileAsync(outFile, MediaTrimmingPreference.Fast, profile)
                         Dim lastLoggedPct As Integer = -1

                         renderOp.Progress = Sub(info, progress)
                                                 Dim pct = CInt(progress)
                                                 If pct > lastLoggedPct Then
                                                     lastLoggedPct = pct
                                                     Dim elapsed = sw.ElapsedMilliseconds
                                                     Dim remSec = If(pct > 0, CInt(((elapsed / (pct / 100.0)) - elapsed) / 1000), 0)
                                                     
                                                     Me.BeginInvoke(Sub()
                                                                        If Not Me.IsDisposed Then
                                                                            pbProgress.Value = pct
                                                                            txtLog.AppendText($"[PROGRESS] {pct}% - {remSec}s left" & vbCrLf)
                                                                        End If
                                                                    End Sub)
                                                 End If
                                             End Sub

                         ' Force clean synchronous block wait sequence
                         renderOp.AsTask().GetAwaiter().GetResult()
                         sw.Stop()

                         Me.BeginInvoke(Sub()
                                            txtLog.AppendText($"[SUCCESS] Render finished in {CInt(sw.Elapsed.TotalSeconds)}s" & vbCrLf)
                                            Process.Start("explorer.exe", $"/select,""{outFile.Path}""")
                                        End Sub)

                     Catch ex As Exception
                         Me.BeginInvoke(Sub() txtLog.AppendText($"[FATAL ERROR] {ex.Message}" & vbCrLf))
                     Finally
                         Me.BeginInvoke(Sub()
                                            btnMerge.Enabled = True
                                        End Sub)
                     End Try
                 End Sub)
    End Sub
End Class
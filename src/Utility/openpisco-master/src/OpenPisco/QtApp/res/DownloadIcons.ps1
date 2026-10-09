

#$user = $env:username
#$webproxy = (get-itemproperty 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Internet Settings').ProxyServer
#$pwd = Read-Host "Password?" -assecurestring
#$proxy = new-object System.Net.WebProxy
#$proxy.Address = $webproxy
#$account = new-object System.Net.NetworkCredential($user,[Runtime.InteropServices.Marshal]::PtrToStringAuto([Runtime.InteropServices.Marshal]::SecureStringToBSTR($pwd)), "")
#$proxy.credentials = $account


Write-Host "Setting Up Proxy"

$proxy = [System.Net.WebRequest]::GetSystemWebProxy()
$proxy.Credentials = [System.Net.CredentialCache]::DefaultCredentials

Write-Host "Downloading"

$client = new-object System.Net.WebClient
$client.proxy = $proxy
$client.DownloadFile("https://storage.googleapis.com/material-icons/external-assets/v4/icons/svg/ic_add_circle_black_24px.svg","ic_add_circle_black_24px.svg")
$client.DownloadFile("https://storage.googleapis.com/material-icons/external-assets/v4/icons/svg/ic_autorenew_black_24px.svg","ic_autorenew_black_24px.svg")
$client.DownloadFile("https://storage.googleapis.com/material-icons/external-assets/v4/icons/svg/ic_cached_black_24px.svg","ic_cached_black_24px.svg")
$client.DownloadFile("https://storage.googleapis.com/material-icons/external-assets/v4/icons/svg/ic_clear_black_24px.svg","ic_clear_black_24px.svg")
$client.DownloadFile("https://storage.googleapis.com/material-icons/external-assets/v4/icons/svg/ic_exit_to_app_black_24px.svg","ic_exit_to_app_black_24px.svg")
$client.DownloadFile("https://storage.googleapis.com/material-icons/external-assets/v4/icons/svg/ic_folder_open_black_24px.svg","ic_folder_open_black_24px.svg")
$client.DownloadFile("https://storage.googleapis.com/material-icons/external-assets/v4/icons/svg/ic_play_arrow_black_24px.svg","ic_play_arrow_black_24px.svg")
$client.DownloadFile("https://storage.googleapis.com/material-icons/external-assets/v4/icons/svg/ic_play_circle_filled_black_24px.svg","ic_play_circle_filled_black_24px.svg")
$client.DownloadFile("https://storage.googleapis.com/material-icons/external-assets/v4/icons/svg/ic_remove_circle_outline_black_24px.svg","ic_remove_circle_outline_black_24px.svg")
$client.DownloadFile("https://storage.googleapis.com/material-icons/external-assets/v4/icons/svg/ic_replay_black_24px.svg","ic_replay_1_black_24px.svg")
#these files are rendered in the wrong way in qt a modificated version are avilable in the sources
#$client.DownloadFile("https://storage.googleapis.com/material-icons/external-assets/v4/icons/svg/ic_replay_30_black_24px.svg","ic_replay_30_black_24px.svg")
#$client.DownloadFile("https://storage.googleapis.com/material-icons/external-assets/v4/icons/svg/ic_replay_10_black_24px.svg","ic_replay_10_black_24px.svg")
#$client.DownloadFile("https://storage.googleapis.com/material-icons/external-assets/v4/icons/svg/ic_replay_5_black_24px.svg","ic_replay_5_black_24px.svg")
$client.DownloadFile("https://storage.googleapis.com/material-icons/external-assets/v4/icons/svg/ic_replay_black_24px.svg","ic_replay_black_24px.svg")
$client.DownloadFile("https://storage.googleapis.com/material-icons/external-assets/v4/icons/svg/ic_vertical_align_center_black_24px.svg","ic_vertical_align_center_black_24px.svg")
$client.DownloadFile("https://storage.googleapis.com/material-icons/external-assets/v4/icons/svg/ic_directions_walk_black_24px.svg","ic_directions_walk_black_24px.svg")
$client.DownloadFile("https://material.io/tools/icons/static/icons/baseline-save_alt-24px.svg")
$client.DownloadFile("https://material.io/tools/icons/static/icons/baseline-layers-24px.svg")
$client.DownloadFile("https://material.io/tools/icons/static/icons/baseline-visibility-24px.svg")
Write-Host "Done"

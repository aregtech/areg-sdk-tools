Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass

dir *.* -Recurse | foreach {  
  # Note that since we are reading and saving to the same file,
  # we need to enclose the command in parenthesis so it fully executes 
  # (reading all content and closing the file) before proceeding
  (Get-Content $_) | Set-Content -Encoding utf8 $_
}

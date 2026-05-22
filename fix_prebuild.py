path = r'D:\Dev\DeferredRendering\DeferredRendering.vcxproj'
with open(path, 'r', encoding='utf-8') as f:
    content = f.read()

# The old PreBuildEvent <Command> ends with just "endlocal" after copy DLL
old_command = '''      <Command>setlocal
if not exist "$(TargetDir)" mkdir "$(TargetDir)"
copy /Y "D:\\Dev\\DeferredRendering\\vendor\\dll\\assimp-vc143-mtd.dll" "$(TargetDir)"
if %errorlevel% neq 0 exit /b 1
endlocal</Command>'''

new_command = '''      <Command>setlocal
if not exist "$(TargetDir)" mkdir "$(TargetDir)"
copy /Y "D:\\Dev\\DeferredRendering\\vendor\\dll\\assimp-vc143-mtd.dll" "$(TargetDir)"
if %errorlevel% neq 0 exit /b 1
for %%f in ("$(ProjectDir)Material\\*.mat") do (
  "$(SolutionDir)Binaries\\$(Platform)\\$(Configuration)\\ShaderCompiler.exe" "%%f" --target opengl4.5 --template-dir "$(ProjectDir)Template" -o CompiledMaterials || exit /b 1
)
endlocal</Command>'''

count = content.count(old_command)
print(f'Found {count} PreBuildEvent(s) to update')

if count == 0:
    # Try without doubled backslashes
    old2 = old_command.replace('\\\\', '\\')
    count = content.count(old2)
    print(f'Trying with single backslashes: {count}')
    if count > 0:
        old_command = old2

if count > 0:
    content = content.replace(old_command, new_command)
    # Update messages
    content = content.replace(
        '<Message>Copy Target Dll file to Outdir </Message>',
        '<Message>Copy DLL + compile all .mat shaders to SPIR-V</Message>'
    )
    with open(path, 'w', encoding='utf-8', newline='\r\n') as f:
        f.write(content)
    print('Done! Updated both PreBuildEvent sections.')
else:
    print('ERROR: Could not find pattern.')
    # Show what close matches exist
    for i, line in enumerate(content.split('\n')):
        if 'PreBuildEvent' in line or 'endlocal' in line:
            print(f'  Line {i+1}: {line.strip()[:100]}')

@echo Running: %1%
@SET LUA_PATH=C:\Users\gordy\AppData\Local\Programs\Lua\bin\lua\?.lua;C:\Users\gordy\AppData\Local\Programs\Lua\bin\lua\?\init.lua;C:\Users\gordy\AppData\Local\Programs\Lua\bin\?.lua;C:\Users\gordy\AppData\Local\Programs\Lua\bin\?\init.lua;C:\Users\gordy\AppData\Local\Programs\Lua\bin\..\share\lua\5.4\?.lua;C:\Users\gordy\AppData\Local\Programs\Lua\bin\..\share\lua\5.4\?\init.lua;.\?.lua;.\?\init.lua;C:\Users\gordy\AppData\Roaming/luarocks/share/lua/5.4/?.lua;C:\Users\gordy\AppData\Roaming/luarocks/share/lua/5.4/?/init.lua
@SET LUA_CPATH=C:\Users\gordy\AppData\Local\Programs\Lua\bin\?.dll;C:\Users\gordy\AppData\Local\Programs\Lua\bin\..\lib\lua\5.4\?.dll;C:\Users\gordy\AppData\Local\Programs\Lua\bin\loadall.dll;.\?.dll;C:\Users\gordy\AppData\Roaming/luarocks/lib/lua/5.4/?.dll
@cd %USERPROFILE%
@lua %1% -l
pause
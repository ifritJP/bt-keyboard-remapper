--./conv.lns
local _moduleObj = {}
local __mod__ = '@conv'
local _lune = {}
if _lune6 then
   _lune = _lune6
end
function _lune.loadstring52( txt, env )
   if not env then
      return load( txt )
   end
   return load( txt, "", "bt", env )
end

function _lune._toStem( val )
   return val
end
function _lune._toInt( val )
   if type( val ) == "number" then
      return math.floor( val )
   end
   return nil
end
function _lune._toReal( val )
   if type( val ) == "number" then
      return val
   end
   return nil
end
function _lune._toBool( val )
   if type( val ) == "boolean" then
      return val
   end
   return nil
end
function _lune._toStr( val )
   if type( val ) == "string" then
      return val
   end
   return nil
end
function _lune._toList( val, toValInfoList )
   if type( val ) == "table" then
      local tbl = {}
      local toValInfo = toValInfoList[ 1 ]
      for index, mem in ipairs( val ) do
         local memval, mess = toValInfo.func( mem, toValInfo.child )
         if memval == nil and not toValInfo.nilable then
            if mess then
              return nil, string.format( "%d.%s", index, mess )
            end
            return nil, index
         end
         tbl[ index ] = memval
      end
      return tbl
   end
   return nil
end
function _lune._toMap( val, toValInfoList )
   if type( val ) == "table" then
      local tbl = {}
      local toKeyInfo = toValInfoList[ 1 ]
      local toValInfo = toValInfoList[ 2 ]
      for key, mem in pairs( val ) do
         local mapKey, keySub = toKeyInfo.func( key, toKeyInfo.child )
         local mapVal, valSub = toValInfo.func( mem, toValInfo.child )
         if mapKey == nil or mapVal == nil then
            if mapKey == nil then
               return nil
            end
            if keySub == nil then
               return nil, mapKey
            end
            return nil, string.format( "%s.%s", mapKey, keySub)
         end
         tbl[ mapKey ] = mapVal
      end
      return tbl
   end
   return nil
end
function _lune._fromMap( obj, map, memInfoList )
   if type( map ) ~= "table" then
      return false
   end
   for index, memInfo in ipairs( memInfoList ) do
      local val, key = memInfo.func( map[ memInfo.name ], memInfo.child )
      if val == nil and not memInfo.nilable then
         return false, key and string.format( "%s.%s", memInfo.name, key) or memInfo.name
      end
      obj[ memInfo.name ] = val
   end
   return true
end

function _lune.loadModule( mod )
   if __luneScript then
      return  __luneScript:loadModule( mod )
   end
   return require( mod )
end

function _lune.__isInstanceOf( obj, class )
   while obj do
      local meta = getmetatable( obj )
      if not meta then
	 return false
      end
      local indexTbl = meta.__index
      if indexTbl == class then
	 return true
      end
      if meta.ifList then
         for index, ifType in ipairs( meta.ifList ) do
            if ifType == class then
               return true
            end
            if _lune.__isInstanceOf( ifType, class ) then
               return true
            end
         end
      end
      obj = indexTbl
   end
   return false
end

function _lune.__Cast( obj, kind, class )
   if kind == 0 then -- int
      if type( obj ) ~= "number" then
         return nil
      end
      if math.floor( obj ) ~= obj then
         return nil
      end
      return obj
   elseif kind == 1 then -- real
      if type( obj ) ~= "number" then
         return nil
      end
      return obj
   elseif kind == 2 then -- str
      if type( obj ) ~= "string" then
         return nil
      end
      return obj
   elseif kind == 3 then -- class
      return _lune.__isInstanceOf( obj, class ) and obj or nil
   end
   return nil
end

if not _lune6 then
   _lune6 = _lune
end
local lnsservlet = _lune.loadModule( 'go/github:com.ifritJP.lnshttpd.src.lns.httpd.lnsservlet' )
local Util = _lune.loadModule( 'go/github:com.ifritJP.LuneScript.src.lune.base.Util' )

local json = require( "go/github:com.ifritJP.lnshttpd.src.lns.httpd.json" )

local base64 = require( "base64" )

local SettingSwitchKey = {}
setmetatable( SettingSwitchKey, { ifList = {Mapping,} } )
function SettingSwitchKey.setmeta( obj )
  setmetatable( obj, { __index = SettingSwitchKey  } )
end
function SettingSwitchKey.new( On, Src, Dst )
   local obj = {}
   SettingSwitchKey.setmeta( obj )
   if obj.__init then
      obj:__init( On, Src, Dst )
   end
   return obj
end
function SettingSwitchKey:__init( On, Src, Dst )

   self.On = On
   self.Src = Src
   self.Dst = Dst
end
function SettingSwitchKey:get_On()
   return self.On
end
function SettingSwitchKey:get_Src()
   return self.Src
end
function SettingSwitchKey:get_Dst()
   return self.Dst
end
function SettingSwitchKey:_toMap()
  return self
end
function SettingSwitchKey._fromMap( val )
  local obj, mes = SettingSwitchKey._fromMapSub( {}, val )
  if obj then
     SettingSwitchKey.setmeta( obj )
  end
  return obj, mes
end
function SettingSwitchKey._fromStem( val )
  return SettingSwitchKey._fromMap( val )
end

function SettingSwitchKey._fromMapSub( obj, val )
   local memInfo = {}
   table.insert( memInfo, { name = "On", func = _lune._toBool, nilable = true, child = {} } )
   table.insert( memInfo, { name = "Src", func = _lune._toInt, nilable = false, child = {} } )
   table.insert( memInfo, { name = "Dst", func = _lune._toInt, nilable = false, child = {} } )
   local result, mess = _lune._fromMap( obj, val, memInfo )
   if not result then
      return nil, mess
   end
   return obj
end


local ConvKeyInfo = {}
setmetatable( ConvKeyInfo, { ifList = {Mapping,} } )
function ConvKeyInfo.setmeta( obj )
  setmetatable( obj, { __index = ConvKeyInfo  } )
end
function ConvKeyInfo.new( On, CondModifierMask, CondModifierResult, Code, ModifierXor )
   local obj = {}
   ConvKeyInfo.setmeta( obj )
   if obj.__init then
      obj:__init( On, CondModifierMask, CondModifierResult, Code, ModifierXor )
   end
   return obj
end
function ConvKeyInfo:__init( On, CondModifierMask, CondModifierResult, Code, ModifierXor )

   self.On = On
   self.CondModifierMask = CondModifierMask
   self.CondModifierResult = CondModifierResult
   self.Code = Code
   self.ModifierXor = ModifierXor
end
function ConvKeyInfo:get_On()
   return self.On
end
function ConvKeyInfo:get_CondModifierMask()
   return self.CondModifierMask
end
function ConvKeyInfo:get_CondModifierResult()
   return self.CondModifierResult
end
function ConvKeyInfo:get_Code()
   return self.Code
end
function ConvKeyInfo:get_ModifierXor()
   return self.ModifierXor
end
function ConvKeyInfo:_toMap()
  return self
end
function ConvKeyInfo._fromMap( val )
  local obj, mes = ConvKeyInfo._fromMapSub( {}, val )
  if obj then
     ConvKeyInfo.setmeta( obj )
  end
  return obj, mes
end
function ConvKeyInfo._fromStem( val )
  return ConvKeyInfo._fromMap( val )
end

function ConvKeyInfo._fromMapSub( obj, val )
   local memInfo = {}
   table.insert( memInfo, { name = "On", func = _lune._toBool, nilable = true, child = {} } )
   table.insert( memInfo, { name = "CondModifierMask", func = _lune._toInt, nilable = false, child = {} } )
   table.insert( memInfo, { name = "CondModifierResult", func = _lune._toInt, nilable = false, child = {} } )
   table.insert( memInfo, { name = "Code", func = _lune._toInt, nilable = false, child = {} } )
   table.insert( memInfo, { name = "ModifierXor", func = _lune._toInt, nilable = false, child = {} } )
   local result, mess = _lune._fromMap( obj, val, memInfo )
   if not result then
      return nil, mess
   end
   return obj
end


local ConfigData = {}
setmetatable( ConfigData, { ifList = {Mapping,} } )
function ConfigData.setmeta( obj )
  setmetatable( obj, { __index = ConfigData  } )
end
function ConfigData.new( SwitchKeys, ConvKeyMap )
   local obj = {}
   ConfigData.setmeta( obj )
   if obj.__init then
      obj:__init( SwitchKeys, ConvKeyMap )
   end
   return obj
end
function ConfigData:__init( SwitchKeys, ConvKeyMap )

   self.SwitchKeys = SwitchKeys
   self.ConvKeyMap = ConvKeyMap
end
function ConfigData:get_SwitchKeys()
   return self.SwitchKeys
end
function ConfigData:get_ConvKeyMap()
   return self.ConvKeyMap
end
function ConfigData:_toMap()
  return self
end
function ConfigData._fromMap( val )
  local obj, mes = ConfigData._fromMapSub( {}, val )
  if obj then
     ConfigData.setmeta( obj )
  end
  return obj, mes
end
function ConfigData._fromStem( val )
  return ConfigData._fromMap( val )
end

function ConfigData._fromMapSub( obj, val )
   local memInfo = {}
   table.insert( memInfo, { name = "SwitchKeys", func = _lune._toList, nilable = false, child = { { func = SettingSwitchKey._fromMap, nilable = false, child = {} } } } )
   table.insert( memInfo, { name = "ConvKeyMap", func = _lune._toMap, nilable = false, child = { { func = _lune._toStr, nilable = false, child = {} }, 
{ func = _lune._toList, nilable = false, child = { { func = ConvKeyInfo._fromMap, nilable = false, child = {} } } } } } )
   local result, mess = _lune._fromMap( obj, val, memInfo )
   if not result then
      return nil, mess
   end
   return obj
end


local ConfigWriter = {}
function ConfigWriter:toint32bin( val )

   return string.format( "%c%c%c%c", (val >> 0 ) & 0xff, (val >> 8 ) & 0xff, (val >> 16 ) & 0xff, (val >> 24 ) & 0xff)
end
function ConfigWriter:toint8bin( val )

   return string.format( "%c", (val >> 0 ) & 0xff)
end
function ConfigWriter:writeInt32( val )

   self.stream:write( self:toint32bin( val ) )
end
function ConfigWriter:writeInt8( val )

   self.stream:write( self:toint8bin( val ) )
end
function ConfigWriter.setmeta( obj )
  setmetatable( obj, { __index = ConfigWriter  } )
end
function ConfigWriter.new( stream )
   local obj = {}
   ConfigWriter.setmeta( obj )
   if obj.__init then
      obj:__init( stream )
   end
   return obj
end
function ConfigWriter:__init( stream )

   self.stream = stream
end


local function countMap( map )

   local count = 0
   for __index, _1 in pairs( map ) do
      count = count + 1
   end
   
   return count
end

local function writeConfigData( stream, conf )

   local writer = ConfigWriter.new(stream)
   
   writer:writeInt32( 1 )
   writer:writeInt32( #conf:get_SwitchKeys() )
   writer:writeInt32( countMap( conf:get_ConvKeyMap() ) )
   writer:writeInt32( 4 * 4 )
   
   local function isValid( val )
   
      return val ~= false
   end
   
   for __index, info in pairs( conf:get_SwitchKeys() ) do
      if isValid( info:get_On() ) then
         writer:writeInt8( info:get_Src() )
         writer:writeInt8( info:get_Dst() )
      end
      
   end
   
   
   
   for key, list in pairs( conf:get_ConvKeyMap() ) do
      local code
      
      if key:find( "^0x" ) then
         code = tonumber( key:sub( 3 ), 16 )
      else
       
         code = tonumber( key, 10 )
      end
      
      if code ~= nil then
         writer:writeInt8( math.floor(code) )
         local num = 0
         for __index, map in pairs( list ) do
            if isValid( map:get_On() ) then
               num = num + 1
            end
            
         end
         
         writer:writeInt8( num )
         for __index, map in pairs( list ) do
            if isValid( map:get_On() ) then
               writer:writeInt8( map:get_CondModifierMask() )
               writer:writeInt8( map:get_CondModifierResult() )
               writer:writeInt8( map:get_Code() )
               writer:writeInt8( map:get_ModifierXor() )
            end
            
         end
         
      end
      
   end
   
end

local function printUsage( command, errMessage )

   if #errMessage > 0 then
      io.stderr:write( string.format( "error: %s \n", errMessage) )
      print( "" )
   end
   
   print( string.format( "usage: %s configfile", command) )
end

local function process( arg )

   if #arg <= 1 then
      printUsage( arg[1], "" )
      return true
   end
   
   local fileObj = io.open( arg[2] )
   if  nil == fileObj then
      local _fileObj = fileObj
   
      printUsage( arg[1], string.format( "failed to open file -- %s", arg[2]) )
      return false
   end
   
   local jsonObj = json.readJsonObj( lnsservlet.luaInStream.new(fileObj) )
   if  nil == jsonObj then
      local _jsonObj = jsonObj
   
      printUsage( arg[1], "failed to read JSON" )
      return false
   end
   
   local conf, mess = ConfigData._fromStem( jsonObj )
   if conf ~= nil then
      local memStream = Util.memStream.new()
      
      writeConfigData( memStream, conf )
      
      print( #memStream:get_txt() )
      print( base64.encode( memStream:get_txt() ) )
   else
      printUsage( arg[1], string.format( "failed to convert to ConfigData. -- %s", mess) )
      return false
   end
   
   
   return true
end

local function __main( arg )

   if process( arg ) then
      return 0
   end
   
   return 1
end
_moduleObj.__main = __main

do
   local loaded, mess = _lune.loadstring52( [=[
if _lune and _lune._shebang then
  return nil
else
  return arg
end
]=] )
   if loaded ~= nil then
      local args = loaded(  )
      do
         local obj = (args )
         if obj ~= nil then
            local work = obj
            local argList = {""}
            do
               local _exp = work[0]
               if _exp ~= nil then
                  argList[1] = _exp
               end
            end
            for key, val in pairs( work ) do
               if key > 0 then
                  table.insert( argList, val )
               end
            end
            __main( argList )
         else
            -- print( "via lnsc" )
         end
      end
   else
      error( mess )
   end
end

return _moduleObj

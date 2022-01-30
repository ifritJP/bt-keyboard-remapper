// This code is transcompiled by LuneScript.
package main
import . "github.com/ifritJP/LuneScript/src/lune/base/runtime_go"
import lnsservlet "github.com/ifritJP/lnshttpd/src/lns/httpd"
import Util "github.com/ifritJP/LuneScript/src/lune/base"
import json "github.com/ifritJP/lnshttpd/src/lns/httpd"
var init_conv bool
var conv__mod__ string
// for 121
func conv_convExp0_664(arg1 []LnsAny) LnsAny {
    return Lns_getFromMulti( arg1, 0 )
}
// for 129
func conv_convExp0_713(arg1 []LnsAny) (LnsAny, LnsAny) {
    return Lns_getFromMulti( arg1, 0 ), Lns_getFromMulti( arg1, 1 )
}
// 51: decl @conv.countMap
func conv_countMap_6_(_env *LnsEnv, _map *LnsMap) LnsInt {
    var count LnsInt
    count = 0
    for range( _map.Items ) {
        count = count + 1
    }
    return count
}

// 59: decl @conv.writeConfigData
func conv_writeConfigData_7_(_env *LnsEnv, stream Lns_oStream,conf *conv_ConfigData) {
    var writer *conv_ConfigWriter
    writer = Newconv_ConfigWriter(_env, stream)
    writer.FP.WriteInt32(_env, 1)
    writer.FP.WriteInt32(_env, conf.FP.Get_SwitchKeys(_env).Len())
    writer.FP.WriteInt32(_env, conv_countMap_6_(_env, conf.FP.Get_ConvKeyMap(_env)))
    writer.FP.WriteInt32(_env, 4 * 4)
    for _, _info := range( conf.FP.Get_SwitchKeys(_env).Items ) {
        info := _info.(conv_SettingSwitchKeyDownCast).Toconv_SettingSwitchKey()
        if writeConfigData__isValid_0_(_env, info.FP.Get_On(_env)){
            writer.FP.WriteInt8(_env, info.FP.Get_Src(_env))
            writer.FP.WriteInt8(_env, info.FP.Get_Dst(_env))
        }
    }
    for _key, _list := range( conf.FP.Get_ConvKeyMap(_env).Items ) {
        key := _key.(string)
        list := _list.(*LnsList)
        var code LnsAny
        if Lns_isCondTrue( Lns_car(_env.GetVM().String_find(key,"^0x", nil, nil))){
            code = Lns_tonumber(_env.GetVM().String_sub(key,3, nil), 16)
        } else { 
            code = Lns_tonumber(key, 10)
        }
        if code != nil{
            code_113 := code.(LnsReal)
            writer.FP.WriteInt8(_env, (LnsInt)(code_113))
            var num LnsInt
            num = 0
            for _, __map := range( list.Items ) {
                _map := __map.(conv_ConvKeyInfoDownCast).Toconv_ConvKeyInfo()
                if writeConfigData__isValid_0_(_env, _map.FP.Get_On(_env)){
                    num = num + 1
                }
            }
            writer.FP.WriteInt8(_env, num)
            for _, __map := range( list.Items ) {
                _map := __map.(conv_ConvKeyInfoDownCast).Toconv_ConvKeyInfo()
                if writeConfigData__isValid_0_(_env, _map.FP.Get_On(_env)){
                    writer.FP.WriteInt8(_env, _map.FP.Get_CondModifierMask(_env))
                    writer.FP.WriteInt8(_env, _map.FP.Get_CondModifierResult(_env))
                    writer.FP.WriteInt8(_env, _map.FP.Get_Code(_env))
                    writer.FP.WriteInt8(_env, _map.FP.Get_ModifierXor(_env))
                }
            }
        }
    }
}

// 108: decl @conv.printUsage
func conv_printUsage_8_(_env *LnsEnv, command string,errMessage string) {
    if len(errMessage) > 0{
        Lns_io_stderr.Write(_env, _env.GetVM().String_format("error: %s \n", []LnsAny{errMessage}))
        Lns_print([]LnsAny{""})
    }
    Lns_print([]LnsAny{_env.GetVM().String_format("usage: %s configfile", []LnsAny{command})})
}

// 116: decl @conv.process
func conv_process_9_(_env *LnsEnv, arg *LnsList) bool {
    if arg.Len() <= 1{
        conv_printUsage_8_(_env, arg.GetAt(1).(string), "")
        return true
    }
    var fileObj Lns_luaStream
    
    {
        _fileObj := conv_convExp0_664(Lns_2DDD(Lns_io_open(arg.GetAt(2).(string), nil)))
        if _fileObj == nil{
            conv_printUsage_8_(_env, arg.GetAt(1).(string), _env.GetVM().String_format("failed to open file -- %s", []LnsAny{arg.GetAt(2).(string)}))
            return false
        } else {
            fileObj = _fileObj.(Lns_luaStream)
        }
    }
    var jsonObj *LnsMap
    
    {
        _jsonObj := json.ReadJsonObj(_env, lnsservlet.NewLnsservlet_luaInStream(_env, fileObj).FP)
        if _jsonObj == nil{
            conv_printUsage_8_(_env, arg.GetAt(1).(string), "failed to read JSON")
            return false
        } else {
            jsonObj = _jsonObj.(*LnsMap)
        }
    }
    var conf LnsAny
    var mess LnsAny
    conf,mess = conv_ConfigData__fromStem_5_(_env, jsonObj,nil)
    if conf != nil{
        conf_134 := conf.(*conv_ConfigData)
        var memStream *Util.Util_memStream
        memStream = Util.NewUtil_memStream(_env)
        conv_writeConfigData_7_(_env, memStream.FP, conf_134)
        Lns_print([]LnsAny{len(memStream.FP.Get_txt(_env))})
        Lns_print([]LnsAny{Encode(_env, memStream.FP.Get_txt(_env))})
    } else {
        conv_printUsage_8_(_env, arg.GetAt(1).(string), _env.GetVM().String_format("failed to convert to ConfigData. -- %s", []LnsAny{mess}))
        return false
    }
    return true
}

// 145: decl @conv.__main
func Conv___main(_env *LnsEnv, arg *LnsList) LnsInt {
    Lns_conv_init( _env )
    if conv_process_9_(_env, arg){
        return 0
    }
    return 1
}

// 68: decl @conv.writeConfigData.isValid
func writeConfigData__isValid_0_(_env *LnsEnv, val LnsAny) bool {
    return val != false
}

// 35: decl @conv.ConfigWriter.toint32bin
func (self *conv_ConfigWriter) toint32bin(_env *LnsEnv, val LnsInt) string {
    return _env.GetVM().String_format("%c%c%c%c", []LnsAny{(val >> 0) & 0xff, (val >> 8) & 0xff, (val >> 16) & 0xff, (val >> 24) & 0xff})
}
// 39: decl @conv.ConfigWriter.toint8bin
func (self *conv_ConfigWriter) toint8bin(_env *LnsEnv, val LnsInt) string {
    return _env.GetVM().String_format("%c", []LnsAny{(val >> 0) & 0xff})
}
// 43: decl @conv.ConfigWriter.writeInt32
func (self *conv_ConfigWriter) WriteInt32(_env *LnsEnv, val LnsInt) {
    self.stream.Write(_env, self.FP.toint32bin(_env, val))
}
// 46: decl @conv.ConfigWriter.writeInt8
func (self *conv_ConfigWriter) WriteInt8(_env *LnsEnv, val LnsInt) {
    self.stream.Write(_env, self.FP.toint8bin(_env, val))
}


// declaration Class -- SettingSwitchKey
type conv_SettingSwitchKeyMtd interface {
    ToMap() *LnsMap
    Get_Dst(_env *LnsEnv) LnsInt
    Get_On(_env *LnsEnv) LnsAny
    Get_Src(_env *LnsEnv) LnsInt
}
type conv_SettingSwitchKey struct {
    On LnsAny
    Src LnsInt
    Dst LnsInt
    FP conv_SettingSwitchKeyMtd
}
func conv_SettingSwitchKey2Stem( obj LnsAny ) LnsAny {
    if obj == nil {
        return nil
    }
    return obj.(*conv_SettingSwitchKey).FP
}
type conv_SettingSwitchKeyDownCast interface {
    Toconv_SettingSwitchKey() *conv_SettingSwitchKey
}
func conv_SettingSwitchKeyDownCastF( multi ...LnsAny ) LnsAny {
    if len( multi ) == 0 { return nil }
    obj := multi[ 0 ]
    if ddd, ok := multi[ 0 ].([]LnsAny); ok { obj = ddd[0] }
    work, ok := obj.(conv_SettingSwitchKeyDownCast)
    if ok { return work.Toconv_SettingSwitchKey() }
    return nil
}
func (obj *conv_SettingSwitchKey) Toconv_SettingSwitchKey() *conv_SettingSwitchKey {
    return obj
}
func Newconv_SettingSwitchKey(_env *LnsEnv, arg1 LnsAny, arg2 LnsInt, arg3 LnsInt) *conv_SettingSwitchKey {
    obj := &conv_SettingSwitchKey{}
    obj.FP = obj
    obj.Initconv_SettingSwitchKey(_env, arg1, arg2, arg3)
    return obj
}
func (self *conv_SettingSwitchKey) Initconv_SettingSwitchKey(_env *LnsEnv, arg1 LnsAny, arg2 LnsInt, arg3 LnsInt) {
    self.On = arg1
    self.Src = arg2
    self.Dst = arg3
}
func (self *conv_SettingSwitchKey) Get_On(_env *LnsEnv) LnsAny{ return self.On }
func (self *conv_SettingSwitchKey) Get_Src(_env *LnsEnv) LnsInt{ return self.Src }
func (self *conv_SettingSwitchKey) Get_Dst(_env *LnsEnv) LnsInt{ return self.Dst }
func (self *conv_SettingSwitchKey) ToMapSetup( obj *LnsMap ) *LnsMap {
    obj.Items["On"] = Lns_ToCollection( self.On )
    obj.Items["Src"] = Lns_ToCollection( self.Src )
    obj.Items["Dst"] = Lns_ToCollection( self.Dst )
    return obj
}
func (self *conv_SettingSwitchKey) ToMap() *LnsMap {
    return self.ToMapSetup( NewLnsMap( map[LnsAny]LnsAny{} ) )
}
func conv_SettingSwitchKey__fromMap_5_(_env,  arg1 LnsAny, paramList []Lns_ToObjParam)(LnsAny, LnsAny){
   return conv_SettingSwitchKey_FromMap( arg1, paramList )
}
func conv_SettingSwitchKey__fromStem_6_(_env,  arg1 LnsAny, paramList []Lns_ToObjParam)(LnsAny, LnsAny){
   return conv_SettingSwitchKey_FromMap( arg1, paramList )
}
func conv_SettingSwitchKey_FromMap( obj LnsAny, paramList []Lns_ToObjParam ) (LnsAny, LnsAny) {
    _,conv,mess := conv_SettingSwitchKey_FromMapSub(obj,false, paramList);
    return conv,mess
}
func conv_SettingSwitchKey_FromMapSub( obj LnsAny, nilable bool, paramList []Lns_ToObjParam ) (bool, LnsAny, LnsAny) {
    var objMap *LnsMap
    if work, ok := obj.(*LnsMap); !ok {
       return false, nil, "no map -- " + Lns_ToString(obj)
    } else {
       objMap = work
    }
    newObj := &conv_SettingSwitchKey{}
    newObj.FP = newObj
    return conv_SettingSwitchKey_FromMapMain( newObj, objMap, paramList )
}
func conv_SettingSwitchKey_FromMapMain( newObj *conv_SettingSwitchKey, objMap *LnsMap, paramList []Lns_ToObjParam ) (bool, LnsAny, LnsAny) {
    if ok,conv,mess := Lns_ToBoolSub( objMap.Items["On"], true, nil); !ok {
       return false,nil,"On:" + mess.(string)
    } else {
       newObj.On = conv
    }
    if ok,conv,mess := Lns_ToIntSub( objMap.Items["Src"], false, nil); !ok {
       return false,nil,"Src:" + mess.(string)
    } else {
       newObj.Src = conv.(LnsInt)
    }
    if ok,conv,mess := Lns_ToIntSub( objMap.Items["Dst"], false, nil); !ok {
       return false,nil,"Dst:" + mess.(string)
    } else {
       newObj.Dst = conv.(LnsInt)
    }
    return true, newObj, nil
}

// declaration Class -- ConvKeyInfo
type conv_ConvKeyInfoMtd interface {
    ToMap() *LnsMap
    Get_Code(_env *LnsEnv) LnsInt
    Get_CondModifierMask(_env *LnsEnv) LnsInt
    Get_CondModifierResult(_env *LnsEnv) LnsInt
    Get_ModifierXor(_env *LnsEnv) LnsInt
    Get_On(_env *LnsEnv) LnsAny
}
type conv_ConvKeyInfo struct {
    On LnsAny
    CondModifierMask LnsInt
    CondModifierResult LnsInt
    Code LnsInt
    ModifierXor LnsInt
    FP conv_ConvKeyInfoMtd
}
func conv_ConvKeyInfo2Stem( obj LnsAny ) LnsAny {
    if obj == nil {
        return nil
    }
    return obj.(*conv_ConvKeyInfo).FP
}
type conv_ConvKeyInfoDownCast interface {
    Toconv_ConvKeyInfo() *conv_ConvKeyInfo
}
func conv_ConvKeyInfoDownCastF( multi ...LnsAny ) LnsAny {
    if len( multi ) == 0 { return nil }
    obj := multi[ 0 ]
    if ddd, ok := multi[ 0 ].([]LnsAny); ok { obj = ddd[0] }
    work, ok := obj.(conv_ConvKeyInfoDownCast)
    if ok { return work.Toconv_ConvKeyInfo() }
    return nil
}
func (obj *conv_ConvKeyInfo) Toconv_ConvKeyInfo() *conv_ConvKeyInfo {
    return obj
}
func Newconv_ConvKeyInfo(_env *LnsEnv, arg1 LnsAny, arg2 LnsInt, arg3 LnsInt, arg4 LnsInt, arg5 LnsInt) *conv_ConvKeyInfo {
    obj := &conv_ConvKeyInfo{}
    obj.FP = obj
    obj.Initconv_ConvKeyInfo(_env, arg1, arg2, arg3, arg4, arg5)
    return obj
}
func (self *conv_ConvKeyInfo) Initconv_ConvKeyInfo(_env *LnsEnv, arg1 LnsAny, arg2 LnsInt, arg3 LnsInt, arg4 LnsInt, arg5 LnsInt) {
    self.On = arg1
    self.CondModifierMask = arg2
    self.CondModifierResult = arg3
    self.Code = arg4
    self.ModifierXor = arg5
}
func (self *conv_ConvKeyInfo) Get_On(_env *LnsEnv) LnsAny{ return self.On }
func (self *conv_ConvKeyInfo) Get_CondModifierMask(_env *LnsEnv) LnsInt{ return self.CondModifierMask }
func (self *conv_ConvKeyInfo) Get_CondModifierResult(_env *LnsEnv) LnsInt{ return self.CondModifierResult }
func (self *conv_ConvKeyInfo) Get_Code(_env *LnsEnv) LnsInt{ return self.Code }
func (self *conv_ConvKeyInfo) Get_ModifierXor(_env *LnsEnv) LnsInt{ return self.ModifierXor }
func (self *conv_ConvKeyInfo) ToMapSetup( obj *LnsMap ) *LnsMap {
    obj.Items["On"] = Lns_ToCollection( self.On )
    obj.Items["CondModifierMask"] = Lns_ToCollection( self.CondModifierMask )
    obj.Items["CondModifierResult"] = Lns_ToCollection( self.CondModifierResult )
    obj.Items["Code"] = Lns_ToCollection( self.Code )
    obj.Items["ModifierXor"] = Lns_ToCollection( self.ModifierXor )
    return obj
}
func (self *conv_ConvKeyInfo) ToMap() *LnsMap {
    return self.ToMapSetup( NewLnsMap( map[LnsAny]LnsAny{} ) )
}
func conv_ConvKeyInfo__fromMap_7_(_env,  arg1 LnsAny, paramList []Lns_ToObjParam)(LnsAny, LnsAny){
   return conv_ConvKeyInfo_FromMap( arg1, paramList )
}
func conv_ConvKeyInfo__fromStem_8_(_env,  arg1 LnsAny, paramList []Lns_ToObjParam)(LnsAny, LnsAny){
   return conv_ConvKeyInfo_FromMap( arg1, paramList )
}
func conv_ConvKeyInfo_FromMap( obj LnsAny, paramList []Lns_ToObjParam ) (LnsAny, LnsAny) {
    _,conv,mess := conv_ConvKeyInfo_FromMapSub(obj,false, paramList);
    return conv,mess
}
func conv_ConvKeyInfo_FromMapSub( obj LnsAny, nilable bool, paramList []Lns_ToObjParam ) (bool, LnsAny, LnsAny) {
    var objMap *LnsMap
    if work, ok := obj.(*LnsMap); !ok {
       return false, nil, "no map -- " + Lns_ToString(obj)
    } else {
       objMap = work
    }
    newObj := &conv_ConvKeyInfo{}
    newObj.FP = newObj
    return conv_ConvKeyInfo_FromMapMain( newObj, objMap, paramList )
}
func conv_ConvKeyInfo_FromMapMain( newObj *conv_ConvKeyInfo, objMap *LnsMap, paramList []Lns_ToObjParam ) (bool, LnsAny, LnsAny) {
    if ok,conv,mess := Lns_ToBoolSub( objMap.Items["On"], true, nil); !ok {
       return false,nil,"On:" + mess.(string)
    } else {
       newObj.On = conv
    }
    if ok,conv,mess := Lns_ToIntSub( objMap.Items["CondModifierMask"], false, nil); !ok {
       return false,nil,"CondModifierMask:" + mess.(string)
    } else {
       newObj.CondModifierMask = conv.(LnsInt)
    }
    if ok,conv,mess := Lns_ToIntSub( objMap.Items["CondModifierResult"], false, nil); !ok {
       return false,nil,"CondModifierResult:" + mess.(string)
    } else {
       newObj.CondModifierResult = conv.(LnsInt)
    }
    if ok,conv,mess := Lns_ToIntSub( objMap.Items["Code"], false, nil); !ok {
       return false,nil,"Code:" + mess.(string)
    } else {
       newObj.Code = conv.(LnsInt)
    }
    if ok,conv,mess := Lns_ToIntSub( objMap.Items["ModifierXor"], false, nil); !ok {
       return false,nil,"ModifierXor:" + mess.(string)
    } else {
       newObj.ModifierXor = conv.(LnsInt)
    }
    return true, newObj, nil
}

// declaration Class -- ConfigData
type conv_ConfigDataMtd interface {
    ToMap() *LnsMap
    Get_ConvKeyMap(_env *LnsEnv) *LnsMap
    Get_SwitchKeys(_env *LnsEnv) *LnsList
}
type conv_ConfigData struct {
    SwitchKeys *LnsList
    ConvKeyMap *LnsMap
    FP conv_ConfigDataMtd
}
func conv_ConfigData2Stem( obj LnsAny ) LnsAny {
    if obj == nil {
        return nil
    }
    return obj.(*conv_ConfigData).FP
}
type conv_ConfigDataDownCast interface {
    Toconv_ConfigData() *conv_ConfigData
}
func conv_ConfigDataDownCastF( multi ...LnsAny ) LnsAny {
    if len( multi ) == 0 { return nil }
    obj := multi[ 0 ]
    if ddd, ok := multi[ 0 ].([]LnsAny); ok { obj = ddd[0] }
    work, ok := obj.(conv_ConfigDataDownCast)
    if ok { return work.Toconv_ConfigData() }
    return nil
}
func (obj *conv_ConfigData) Toconv_ConfigData() *conv_ConfigData {
    return obj
}
func Newconv_ConfigData(_env *LnsEnv, arg1 *LnsList, arg2 *LnsMap) *conv_ConfigData {
    obj := &conv_ConfigData{}
    obj.FP = obj
    obj.Initconv_ConfigData(_env, arg1, arg2)
    return obj
}
func (self *conv_ConfigData) Initconv_ConfigData(_env *LnsEnv, arg1 *LnsList, arg2 *LnsMap) {
    self.SwitchKeys = arg1
    self.ConvKeyMap = arg2
}
func (self *conv_ConfigData) Get_SwitchKeys(_env *LnsEnv) *LnsList{ return self.SwitchKeys }
func (self *conv_ConfigData) Get_ConvKeyMap(_env *LnsEnv) *LnsMap{ return self.ConvKeyMap }
func (self *conv_ConfigData) ToMapSetup( obj *LnsMap ) *LnsMap {
    obj.Items["SwitchKeys"] = Lns_ToCollection( self.SwitchKeys )
    obj.Items["ConvKeyMap"] = Lns_ToCollection( self.ConvKeyMap )
    return obj
}
func (self *conv_ConfigData) ToMap() *LnsMap {
    return self.ToMapSetup( NewLnsMap( map[LnsAny]LnsAny{} ) )
}
func conv_ConfigData__fromMap_4_(_env,  arg1 LnsAny, paramList []Lns_ToObjParam)(LnsAny, LnsAny){
   return conv_ConfigData_FromMap( arg1, paramList )
}
func conv_ConfigData__fromStem_5_(_env,  arg1 LnsAny, paramList []Lns_ToObjParam)(LnsAny, LnsAny){
   return conv_ConfigData_FromMap( arg1, paramList )
}
func conv_ConfigData_FromMap( obj LnsAny, paramList []Lns_ToObjParam ) (LnsAny, LnsAny) {
    _,conv,mess := conv_ConfigData_FromMapSub(obj,false, paramList);
    return conv,mess
}
func conv_ConfigData_FromMapSub( obj LnsAny, nilable bool, paramList []Lns_ToObjParam ) (bool, LnsAny, LnsAny) {
    var objMap *LnsMap
    if work, ok := obj.(*LnsMap); !ok {
       return false, nil, "no map -- " + Lns_ToString(obj)
    } else {
       objMap = work
    }
    newObj := &conv_ConfigData{}
    newObj.FP = newObj
    return conv_ConfigData_FromMapMain( newObj, objMap, paramList )
}
func conv_ConfigData_FromMapMain( newObj *conv_ConfigData, objMap *LnsMap, paramList []Lns_ToObjParam ) (bool, LnsAny, LnsAny) {
    if ok,conv,mess := Lns_ToListSub( objMap.Items["SwitchKeys"], false, []Lns_ToObjParam{Lns_ToObjParam{
            conv_SettingSwitchKey_FromMapSub, false,nil}}); !ok {
       return false,nil,"SwitchKeys:" + mess.(string)
    } else {
       newObj.SwitchKeys = conv.(*LnsList)
    }
    if ok,conv,mess := Lns_ToLnsMapSub( objMap.Items["ConvKeyMap"], false, []Lns_ToObjParam{Lns_ToObjParam{
            Lns_ToStrSub, false,nil},Lns_ToObjParam{
            Lns_ToListSub, false,[]Lns_ToObjParam{Lns_ToObjParam{
                    conv_ConvKeyInfo_FromMapSub, false,nil}}}}); !ok {
       return false,nil,"ConvKeyMap:" + mess.(string)
    } else {
       newObj.ConvKeyMap = conv.(*LnsMap)
    }
    return true, newObj, nil
}

// declaration Class -- ConfigWriter
type conv_ConfigWriterMtd interface {
    toint32bin(_env *LnsEnv, arg1 LnsInt) string
    toint8bin(_env *LnsEnv, arg1 LnsInt) string
    WriteInt32(_env *LnsEnv, arg1 LnsInt)
    WriteInt8(_env *LnsEnv, arg1 LnsInt)
}
type conv_ConfigWriter struct {
    stream Lns_oStream
    FP conv_ConfigWriterMtd
}
func conv_ConfigWriter2Stem( obj LnsAny ) LnsAny {
    if obj == nil {
        return nil
    }
    return obj.(*conv_ConfigWriter).FP
}
type conv_ConfigWriterDownCast interface {
    Toconv_ConfigWriter() *conv_ConfigWriter
}
func conv_ConfigWriterDownCastF( multi ...LnsAny ) LnsAny {
    if len( multi ) == 0 { return nil }
    obj := multi[ 0 ]
    if ddd, ok := multi[ 0 ].([]LnsAny); ok { obj = ddd[0] }
    work, ok := obj.(conv_ConfigWriterDownCast)
    if ok { return work.Toconv_ConfigWriter() }
    return nil
}
func (obj *conv_ConfigWriter) Toconv_ConfigWriter() *conv_ConfigWriter {
    return obj
}
func Newconv_ConfigWriter(_env *LnsEnv, arg1 Lns_oStream) *conv_ConfigWriter {
    obj := &conv_ConfigWriter{}
    obj.FP = obj
    obj.Initconv_ConfigWriter(_env, arg1)
    return obj
}
func (self *conv_ConfigWriter) Initconv_ConfigWriter(_env *LnsEnv, arg1 Lns_oStream) {
    self.stream = arg1
}

func Lns_conv_init(_env *LnsEnv) {
    if init_conv { return }
    init_conv = true
    conv__mod__ = "@conv"
    Lns_InitMod()
    lnsservlet.Lns_lnsservlet_init(_env)
    Util.Lns_Util_init(_env)
}
func init() {
    init_conv = false
}

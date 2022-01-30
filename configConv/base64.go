package main

import (
	"encoding/base64"

	. "github.com/ifritJP/LuneScript/src/lune/base/runtime_go"
)

func Encode(_env *LnsEnv, txt string) string {
	return base64.StdEncoding.EncodeToString([]byte(txt))
}

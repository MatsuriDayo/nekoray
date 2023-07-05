package v2ray_sip

import (
	"fmt"

	"github.com/xtls/xray-core/common/errors"
)

type errPathObjHolder struct{}

func newError(values ...interface{}) *errors.Error {
	return errors.New(values...).WithPathObj(errPathObjHolder{})
}

func newErrorf(format string, a ...interface{}) *errors.Error {
	return errors.New(fmt.Sprintf(format, a)).WithPathObj(errPathObjHolder{})
}

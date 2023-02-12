package plugin

import (
	"fmt"

	"github.com/v2fly/v2ray-core/v5/common/errors"
)

type errPathObjHolder struct{}

func newError(values ...interface{}) *errors.Error {
	return errors.New(values...).WithPathObj(errPathObjHolder{})
}

func newErrorf(format string, a ...interface{}) *errors.Error {
	return errors.New(fmt.Sprintf(format, a)).WithPathObj(errPathObjHolder{})
}

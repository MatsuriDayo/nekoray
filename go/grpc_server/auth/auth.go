package auth

import (
	"context"

	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/metadata"
	"google.golang.org/grpc/status"
)

// Authenticator exposes a function for authenticating requests.
type Authenticator struct {
	Token string
}

// Authenticate checks that a token exists and is valid. It stores the user
// metadata in the returned context and removes the token from the context.
func (a Authenticator) Authenticate(ctx context.Context) (newCtx context.Context, err error) {
	auth, err := extractHeader(ctx, "nekoray_auth")
	if err != nil {
		return ctx, err
	}

	if auth != a.Token {
		return ctx, status.Error(codes.Unauthenticated, "invalid token")
	}

	return purgeHeader(ctx, "nekoray_auth"), nil
}

func extractHeader(ctx context.Context, header string) (string, error) {
	md, ok := metadata.FromIncomingContext(ctx)
	if !ok {
		return "", status.Error(codes.Unauthenticated, "no headers in request")
	}

	authHeaders, ok := md[header]
	if !ok {
		return "", status.Error(codes.Unauthenticated, "no header in request")
	}

	if len(authHeaders) != 1 {
		return "", status.Error(codes.Unauthenticated, "more than 1 header in request")
	}

	return authHeaders[0], nil
}

func purgeHeader(ctx context.Context, header string) context.Context {
	md, _ := metadata.FromIncomingContext(ctx)
	mdCopy := md.Copy()
	mdCopy[header] = nil
	return metadata.NewIncomingContext(ctx, mdCopy)
}

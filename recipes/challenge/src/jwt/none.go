package jwt

// Implements the none signing method.  This is required by the spec
// but you probably should never use it.
var SigningMethodNone *signingMethodNone

type signingMethodNone struct{}

func init() {
	SigningMethodNone = &signingMethodNone{}

	RegisterSigningMethod(SigningMethodNone.Alg(), func() SigningMethod {
		return SigningMethodNone
	})
}

func (m *signingMethodNone) Alg() string {
	return "none"
}

func (m *signingMethodNone) Verify(signingString, signature string, key interface{}) (err error) {
	// If signing method is none, signature must be an empty string
	if signature != "" {
		return NewValidationError(
			"'none' signing method with non-empty signature",
			ValidationErrorSignatureInvalid,
		)
	}

	// Accept 'none' signing method.
	return nil
}

func (m *signingMethodNone) Sign(signingString string, key interface{}) (string, error) {
	return "", nil
}

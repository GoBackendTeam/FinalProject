package store

import "github.com/google/uuid"

func newID() string { return uuid.NewString() }

// NewID 對外暴露的 UUID 產生器(submission operatorId 等使用)。
func NewID() string { return newID() }

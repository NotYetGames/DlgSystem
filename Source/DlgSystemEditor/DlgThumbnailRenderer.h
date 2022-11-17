// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "ThumbnailRendering/DefaultSizedThumbnailRenderer.h"
#include "Runtime/Launch/Resources/Version.h"

#include "DlgThumbnailRenderer.generated.h"

// Useful if you want to have fancy thumbnail previews.
UCLASS()
class DLGSYSTEMEDITOR_API UDlgThumbnailRenderer : public UDefaultSizedThumbnailRenderer
{
	GENERATED_BODY()

public:
	UDlgThumbnailRenderer(const FObjectInitializer& ObjectInitializer);

	// UThumbnailRenderer interface
// #if NY_ENGINE_VERSION >= 425
// 	void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Target, FCanvas* Canvas, bool bAdditionalViewFamily) override;
// #else
// 	void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Target, FCanvas* Canvas,) override;
// #endif
};

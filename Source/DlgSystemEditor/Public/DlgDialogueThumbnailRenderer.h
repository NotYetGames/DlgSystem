// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "ThumbnailRendering/DefaultSizedThumbnailRenderer.h"

#include "DlgDialogueThumbnailRenderer.generated.h"

// Useful if you want to have fancy thumbnail previews.
UCLASS()
class DLGSYSTEMEDITOR_API UDlgDialogueThumbnailRenderer : public UDefaultSizedThumbnailRenderer
{
	GENERATED_BODY()

public:
	UDlgDialogueThumbnailRenderer(const FObjectInitializer& ObjectInitializer);

	// UThumbnailRenderer interface
// #if ENGINE_MINOR_VERSION >= 25
// 	void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Target, FCanvas* Canvas, bool bAdditionalViewFamily) override;
// #else
// 	void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Target, FCanvas* Canvas,) override;
// #endif
};

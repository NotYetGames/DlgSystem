// Copyright 2017-2018 Csaba Molnar, Daniel Butum
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
	void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Target, FCanvas* Canvas) override;
	// End of UThumbnailRenderer interface
};

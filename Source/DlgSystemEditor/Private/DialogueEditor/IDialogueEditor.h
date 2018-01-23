// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "Toolkits/AssetEditorToolkit.h"

class IDialogueEditor : public FAssetEditorToolkit
{
public:
	/** Get the currently selected set of nodes */
	virtual const TSet<UObject*> GetSelectedNodes() const = 0;

	/** Get the bounding area for the currently selected nodes
	 *
	 * @param Rect Final output bounding area, including padding
	 * @param Padding An amount of padding to add to all sides of the bounds
	 *
	 * @return false if nothing is selected
	 */
	virtual bool GetBoundsForSelectedNodes(class FSlateRect& Rect, float Padding) const = 0;

	/** Refreshes the details panel with the Dialogue */
	virtual void RefreshDetailsView() = 0;

	/** Refresh the viewport and property/details panel. */
	virtual void Refresh() = 0;

	/** Useful for setting the last target node on drop operations. */
	virtual UDialogueGraphNode_Edge* GetLastTargetGraphEdgeBeforeDrag() const = 0;
	virtual void SetLastTargetGraphEdgeBeforeDrag(UDialogueGraphNode_Edge* InEdge) = 0;

	/** Jump selection to the selected object. */
	virtual void JumpToObject(const UObject* Object) = 0;
};

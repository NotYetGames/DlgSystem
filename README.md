# DlgSystem

## Interacting `UObject`

### Tips
- When comparing property names from `FPropertyChangedEvent` (handled by `PostEditChangeProperty` for example) use the `GET_MEMBER_NAME_CHECKED` instead of `TEXT`.
eg: `PropertyChangedEvent.Property->GetFName()  = GET_MEMBER_NAME_CHECKED(UClassChildofUObject, PropertyInClass)`

### Gotchas
- If you override the `PostEditChangeProperty` method this will be the one that gets called when a property changes.
But if you also override `PostEditChangeChainProperty` (without calling `Super`) this will be the ONLY METHOD called, no the previous one (`PostEditChangeProperty`)

## Editor explanations of types that you can extend to make your plugin more awesome

### UActorFactory
When you have an asset in the content browser and you want to drag it out on the level editor, how should that happen?

### IComponentAssetBroker
When you drag from the content browser to the blueprint how does it know to go from the asset to the component in the blueprint?

### UFactory
Create a new class from "new" or "import"

### FAssetTypeActions_Base or IAssetTypeActions
How the editor interacts with that asset in the content browser:
- color that shows at the bottom
- category it will show in (in filters and create new)
- what editor is opened

You must create a UFactory for this object type if you want it to appear in the new or import menu.

### UThumbnailRenderer or UDefaultSizedThumbnailRenderer
The default icon that you see for the asset instead of the asset color and name which is the default.

### FEditorViewportClient
Viewport client for editor viewports. Contains common functionality for camera movement, rendering debug information.

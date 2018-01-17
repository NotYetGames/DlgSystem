# DlgSystem (Dialogue System) Plugin

Features:
- a
- b
- c

See Wiki for tutorials on how to install. TODO.

## Conventions

### Code Style

The code style follows the one from the [UE Coding Standard](https://docs.unrealengine.com/latest/INT/Programming/Development/CodingStandard/)
with the additional changes located in the [`.editorconfig`](.editorconfig).

### Commit messages

Every commit message that has `[C]` means that after updating to that commit, a recompile is required.
Eg: `[C] Added awesome Dialogue feature`

Every commit message that has `[G]` means that after updating to that commit the solution files should be regenerated (and recompiled, *obvious*).
(`Right click on project file` -> `Generate Visual Studio project files`) (or from the command line regenerate the project).

Eg: `[G] New Dialogue Editor mode`

Each engine version update has `[4.xx]` tag. Source may or may not be compatible with older versions anymore

Eg: `[4.18][C] Engine update`


## Tips
- When comparing property names from `FPropertyChangedEvent` (handled by `PostEditChangeProperty` for example) use the `GET_MEMBER_NAME_CHECKED` instead of `TEXT`.
eg:
```cpp
PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UClassChildOfUObject, PropertyInClass)
```

## Gotchas
- If you override the `PostEditChangeProperty` method this will be the one that gets called when a property changes.
But if you also override `PostEditChangeChainProperty` (without calling `Super`) this will be the ONLY METHOD called, no the previous one (`PostEditChangeProperty`)

# XBE metadata compatibility

The independent XBE parser recognizes the original Xbox XBEH header,
reads image base, image/header sizes, section-table pointer, encoded
entrypoint and encoded kernel thunk address, and validates section bounds.
This permits future loaders to inspect section layout without loading
or executing untrusted guest code.

No decrypted entrypoint, imports, relocations, guest memory mapping,
kernel execution or XBE game compatibility is claimed. The Cxbx-Reloaded
submodule remains available for future integration of its complete loader.

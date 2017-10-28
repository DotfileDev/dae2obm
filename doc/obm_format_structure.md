# OBM format structure

General overview:

```c
struct obm {
  char    header[4];                      // Should be equal to "OBMF".
  uint8_t meshes_count;                   // OBM format supports up to 255 meshes per file.
  Mesh    meshes[meshes_count];
}
```

For each mesh:

```c
struct mesh {
  uint8   present_attributes;             // 0: positions present, 1: uvs present, 2: normals present, 3: colors present.
  uint32  positions_count;                // Number of position vectors (each vector is made of 3 floats!).
  uint32  attributes_count;               // Number of attribute vectors.
  uint32  position_indices_count;
  uint32  attribute_indices_count;
  vector3 positions[positions_count];
  vector2 uvs[attributes_count];          // If present.
  vector3 normals[attributes_count];      // If present.
  vector3 colors[attributes_count];       // If present.
  uint32  position_indices[position_indices_count];   // If present.
  uint32  uv_indices[attribute_indices_count];        // If present.
  uint32  normal_indices[attribute_indices_count];    // If present.
  uint32  color_indices[attribute_indices_count];     // If present.
}
```

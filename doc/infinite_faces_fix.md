# Boundaryless, unbounded faces: investigation and presentation fix

## Result and source baselines

The long-line and fit-all problem is reproducible in unmodified Mayo `develop`,
commit `8e69135dd5259a6a2b77568fcf630f969f408762` (version 0.11.0, fetched
2026-10-23), with Open CASCADE 7.9.3. A synthetic box plus an untrimmed infinite
cylinder is sufficient. An infinite cone reproduces the same class of failure.

The production change affects four files, adding 118 lines and removing three:

- `src/graphics/graphics_shape_object_driver.cpp`
- `src/graphics/graphics_utils.cpp`
- `src/graphics/graphics_utils.h`
- `src/gui/gui_document.cpp`

It changes presentation and selection only. The original XDE shapes, labels,
component references, locations, colors and visibility attributes are retained.
No OCCT source modification or model healing is required.

Validation used Ubuntu 24.04.3, GCC 13.3, Qt 5.15.13, OCCT 7.9.3, Xvfb and
Mesa llvmpipe. Both original and patched Mayo GUI applications were compiled
and run. Builds used Release configuration with `-O0 -DNDEBUG`; these are
correctness checks, not performance measurements. A Windows executable was not
built or run. A separate production-only backport applies cleanly to Mayo
v0.10.0, commit `f3edd21e77e1f597a65583e9af5beba06a0849ad`; that older version
was not compiled in this investigation.

## Input geometry and rendering are separate issues

The synthetic STEP files contain a face of this form:

```step
#388 = ADVANCED_FACE('Unbounded cylinder test face',(),#403,.T.);
```

There are seven imported faces: six on an 80 x 60 x 40 box and one face with
no wire, no triangulation and an infinite natural parameter domain. Cylinder
and cone bounds are U = [0, 2*pi], V = [-2e100, 2e100]. The plane has both U
and V = [-2e100, 2e100]. These domains are present in the imported geometry;
the viewer does not create them.

The visible long lines come from OCCT's display fallback:

1. Mayo creates an `XCAFPrs_AISObject` and disables automatic triangulation.
2. `AIS_ColoredShape` shaded presentation delegates untriangulated faces to
   `StdPrs_ShadedShape::AddWireframeForFacesWithoutTriangles`.
3. `wireframeNoTriangFacesFromShape` temporarily sets the U and V isoline counts
   to five and calls `StdPrs_WFShape`.
4. `StdPrs_Isolines` can use the analytic surface when no triangulation exists,
   even when `IsoOnTriangulation` is enabled. With no trimming wires,
   `BRepTools::UVBounds` falls back to the natural surface domain.
5. The infinite-domain drawing fallback generates very large finite line
   segments. OCCT's default `MaximalParameterValue` is 500000; the actual
   presentation extents depend on the curve and the range-expansion logic.
   The renderer does not simply send coordinates of magnitude 2e100 to the GPU.

Those line segments contribute to the presentation bounding box. Mayo's
`GraphicsUtils::AisObject_boundingBox()` calls the output-argument overload
`BoundingBox(Bnd_Box&)`, which reads presentation bounds. After the unwanted
presentation is suppressed, ordinary fit-all uses finite bounds. Replacing
the shape or overriding its raw geometric bounding box is unnecessary.

The default plane case is different: `Prs3d_Drawer` defaults `IsoOnPlane` to
false, and `StdPrs_WFShape` skips planar isolines unless it is enabled. The
unmodified plane sample therefore has normal fit-all in this configuration.
It still has an infinite domain, and its unfiltered whole-object selection can
cover an excessively large area. Plane coverage tests the classification and
selection policy, without claiming that its default screenshot has the same
long lines as the cylinder.

## Display policy

Suppress a face in the presentation only when all of these conditions hold:

- It has no topological wire.
- It has an analytic/support surface.
- Its active triangulation is absent or contains no triangles.
- At least one U/V bound is infinite according to `Precision::IsInfinite`.

This uses OCCT's parameter-domain infinity convention. It does not use world
coordinates, a model-size cutoff, or edge lengths. The regression cases retain
spheres, tori, Bezier surfaces and rectangular-trimmed surfaces with finite
natural domains, ordinary trimmed cylinder/plane faces, a finite plane spanning
[-1e8, 1e8], mesh-only faces, and an infinite support surface with a real mesh.
Independent edges and wires keep their normal presentation.

The custom `XCAFPrs_AISObject` subclass first invokes the base `DispatchStyles`,
then reapplies the policy using `CustomAspects(face)->SetHidden(true)`.
Reapplying is essential: OCCT clears its custom-aspect map during style dispatch.
Original finite-face colors and STEP visibility attributes continue to be
handled by the base implementation.

An intentionally infinite, unmeshed and boundaryless face is consequently
omitted from normal display and face picking. A product consisting solely of
such faces appears empty but remains in the model tree and original document.
This patch adds no user preference or finite reference-surface visualization.
A future explicit display mode could visualize those surfaces within a
user-controlled finite domain while keeping them out of ordinary fit-all.

## Selection and instance highlighting

Simply hiding the faces was not sufficient for Mayo's actual selection paths.
The bounding-box measurement tool activates OCCT compound selection (mode 8).
For compound roots, OCCT can build sensitive geometry directly from the raw
aggregate, bypassing hidden child drawers. For shell/solid roots containing a
hidden child, it can instead recurse looking for a compound and find none.

For affected objects, mode 8 now reuses `AIS_ColoredShape::ComputeSelection`
mode 0. This retains the original whole-shape owner and builds sensitivity from
the visible geometry. The selection is still stored and activated as mode 8.
Objects without faces matching the policy use the existing path.

Connected assembly instances require another small correction. OCCT marks
every copied owner from a nonzero mode as decomposed. Even when that owner is
the complete source shape, its highlight is then rebuilt from the raw BREP,
losing hidden-face styles. A Mayo connected-object factory clears the
decomposition flag only for mode-8 owners equal to the complete source shape.
The original owner shape, instance transform and genuine face-subshape owners
remain intact. Whole-object highlighting can reuse the filtered presentation.

This problem was observed in a compiled intermediate hidden-only patch:
cylinder instance highlighting extended approximately from Z = -332652 to
332683, and the plane selection range extended to roughly 1000 x 1000 instead
of the finite test box. The final direct/instance tests cover cylinder, plane,
cone, nested compounds, and shell/solid roots.

The geometric bounding-box measurement itself remains a separate limitation.
`MeasureToolBRep::brepBoundingBox()` applies `BRepBndLib::AddOptimal` to the
original geometry, including the infinite face. It can still yield an
unbounded/huge measurement or fail when constructing its measurement graphic.
The mode-8 tests verify picking and highlight bounds; they do not claim to
repair or validate that complete measurement workflow.

## Measured before/after behavior

The diagnostic executable uses the production Mayo shape driver and a real
OCCT OpenGL view. Both phases use the same STEP files, mesh settings, fixed
camera and fit-all procedure. Values below are model units in the initial
shaded-with-edges mode:

| Sample | Original presentation dimensions | Patched presentation dimensions | Original fit camera scale | Patched scale |
| --- | --- | --- | ---: | ---: |
| Cylinder | 80 x 60 x 665334 | 80 x 60 x 40 | 562900.089383 | 87.724356 |
| Cone | 196649.9375 x 196649.9375 x 635617.84375 | 80 x 60 x 40 | 690727.974171 | 87.724356 |
| Plane, default planar isolines off | 80 x 60 x 40 | 80 x 60 x 40 | 87.724356 | 87.724356 |

For every sample the patched dimensions remain 80 x 60 x 40 through shaded,
shaded-with-edges, wireframe, return to shaded-with-edges, and full style
refresh. Actual Mayo GUI screenshots additionally demonstrate the failed
original automatic framing and the restored fit in the final application.
The diagnostic images are application renderings, not generated illustrations.

Meshing occurs once before the preservation snapshot, so normal initial mesh
generation is not confused with a geometry change caused by display. Complete
BREP serialization, including triangulations, is byte-identical before/after
display and between original/patched runs:

| Sample | SHA-256 of the complete meshed BREP |
| --- | --- |
| Cylinder | `8d57ed0491bc1f82347a38c1f7a88b91df0aa745faea89adb2d0ffa4f214fc0f` |
| Plane | `1467b6a2bd0e398675c6310425bbea50e5cafb6b1d3a09b148198cba4e490f87` |
| Cone | `127b05eb59699a60e6798e2cd54f3c4015463cb078ade42dfb2c10eba1e3cdac` |

These hashes describe the verification BREP snapshots, not the unmeshed BREP
samples generated alongside STEP. The patch leaves import/export code alone.
OCCT's standard STEP exporter already skips a face without an outer wire in
`TopoDSToStep_MakeStepFace`; preserving the in-memory geometry does not add
STEP round-trip support for that exporter limitation.

## Build and reproduce

The full patch includes tests and two optional executables enabled by
`Mayo_BuildTests=ON`. The production-only patch is sufficient for the viewer fix.
Apply the appropriate patch to its stated source baseline, then build against
your installed Qt and OCCT:

```sh
git clone https://github.com/fougue/mayo.git
cd mayo
git checkout 8e69135dd5259a6a2b77568fcf630f969f408762
git apply --check /path/to/mayo-infinite-faces-full.patch
git apply /path/to/mayo-infinite-faces-full.patch
cmake -S . -B ../mayo-build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DOpenCASCADE_DIR=/path/to/occt-install/lib/cmake/opencascade \
  -DMayo_BuildTests=ON -DMayo_BuildPluginAssimp=OFF
cmake --build ../mayo-build --parallel
ctest --test-dir ../mayo-build --output-on-failure
../mayo-build/mayo-repro-infinite-faces ../repro
../mayo-build/mayo ../repro/box-with-unbounded-cylinder.step
../mayo-build/mayo-inspect-infinite-faces \
  ../repro/box-with-unbounded-cylinder.step ../inspection all
```

On a headless Linux machine, run the graphical commands under a working Xvfb
display, with the OCCT shared libraries available to the dynamic loader. This
investigation used `DISPLAY=:99`, `LIBGL_ALWAYS_SOFTWARE=1`, and an
`LD_LIBRARY_PATH` pointing to the local OCCT installation. On Windows, use the
corresponding CMake generator, Qt directory and OCCT runtime DLL paths; no
Windows test result is asserted here.

The generator constructs the unbounded BREP face directly with `BRep_Builder`
to avoid accidentally introducing a seam or natural boundary wire. To create
a STEP file with the same characteristic, it transfers a bounded placeholder,
finds its `StepShape_AdvancedFace` through `STEPConstruct::FindEntity`, empties
that STEP entity's bounds list, and writes the file. This is required because
the ordinary STEP exporter omits the direct boundaryless face. It writes three
bounded controls as well. It re-imports each unbounded STEP and asserts seven
faces, exactly one boundaryless face, and an infinite parameter domain.
All samples use only a synthetic box and analytic surfaces, with no data from
the private industrial assembly.

To reproduce the original comparison, use the same diagnostic source and
sample files, restore the four production files from the baseline commit in
a separate checkout, and build `mayo` and `mayo-inspect-infinite-faces` there.
The new graphics regression cases intentionally fail on the original behavior;
they are not needed to build the original diagnostic executable.

## Regression results and evidence files

The final full CTest run passes all six targets: base, IO, graphics, synthetic
STEP round-trip, measure, and app. Graphics reports 33 passed, 0 failed,
0 skipped, including initialization/cleanup; 24 data-expanded cases are newly
added for this fix. Coverage includes:

- cylinder/plane/cone classification, display modes and style refresh;
- retained finite natural domains, trimmed and very large finite faces;
- mesh-only and mesh-bearing analytic faces, independent edges/wires;
- original shape identity, complete BREP equality, color and visibility;
- assembly references and placement;
- whole-object and face selection, compound-mode sensitivity and highlighting;
- connected instances, selection rebuild, and shell/solid root cases.

The delivery bundle contains the raw PNGs and `stats.json` for each surface and
both phases under `verification/`; final application screenshots under
`screenshots/`; and build, CTest, graphics, generation and backport-application
logs under `logs/`. `SHA256SUMS.txt` identifies all bundled files. The separate
v0.10.0 patch has passed application/reverse-application checks, while runtime
verification applies to the current-source build described above. OCCT 7.4
headers were checked for the new virtual hooks and owner APIs; only OCCT 7.9.3
was compiled and run.

## Primary source references

- [Mayo original shape driver](https://github.com/fougue/mayo/blob/8e69135dd5259a6a2b77568fcf630f969f408762/src/graphics/graphics_shape_object_driver.cpp)
- [Mayo graphics bounding-box helper](https://github.com/fougue/mayo/blob/8e69135dd5259a6a2b77568fcf630f969f408762/src/graphics/graphics_utils.cpp)
- [Mayo measurement selection and geometric bounding box](https://github.com/fougue/mayo/blob/8e69135dd5259a6a2b77568fcf630f969f408762/src/measure/measure_tool_brep.cpp)
- [OCCT 7.9.3 shaded fallback](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/StdPrs/StdPrs_ShadedShape.cxx)
- [OCCT 7.9.3 wireframe and planar-isoline gate](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/StdPrs/StdPrs_WFShape.cxx)
- [OCCT 7.9.3 isoline calculation](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/StdPrs/StdPrs_Isolines.cxx)
- [OCCT 7.9.3 drawer defaults](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/Prs3d/Prs3d_Drawer.cxx)
- [OCCT 7.9.3 XCAF style dispatch](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/XCAFPrs/XCAFPrs_AISObject.cxx)
- [OCCT 7.9.3 colored-shape selection](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/AIS/AIS_ColoredShape.cxx)
- [OCCT 7.9.3 connected selection owners](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/AIS/AIS_ConnectedInteractive.cxx)
- [OCCT 7.9.3 BREP owner highlighting](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/StdSelect/StdSelect_BRepOwner.cxx)
- [OCCT 7.9.3 STEP face exporter](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/TopoDSToStep/TopoDSToStep_MakeStepFace.cxx)

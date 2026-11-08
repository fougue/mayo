# Specifica — Modulo "Analisi Fori" per il fork di Mayo

Versione 1 — 2026-11-08 — Autore: Paolo (officina stampi) con Claude

---

## 0. Obiettivo in una riga

Seleziono un solido → il programma trova tutte le feature cilindriche concave (fori filettati, passanti, ciechi, lamati, acqua, spine…) → le vedo in una tabella con Ø e lunghezza → nella vista 3D il resto del pezzo è trasparente e i fori sono colorati per gruppo.

Riferimento mentale: l'analisi fori di WorkNC. Deve essere **affidabile prima che furba**: meglio un foro "Non classificato" che uno classificato male.

---

## 1. Contesto e vincoli

- **Base:** fork di Mayo (https://github.com/fougue/mayo), C++17, Qt 5/6, OpenCASCADE ≥ 7.6, licenza BSD-2. Mantenere la licenza e il copyright originale.
- **Piattaforma target:** Windows 10/11 x64, workstation CAD/CAM dell'officina.
- **Dati in ingresso:** STEP (AP203/AP214/AP242) di stampi e piastre dei clienti, spesso assiemi grandi, a volte con facce colorate (ogni cliente col suo codice colori), a volte tutto grigio.
- **Unità:** mm. Verificare che l'import converta in mm (`xstep.cascade.unit`), anche se lo STEP è in pollici.
- **Non toccare** il modulo `src/measure/` in questo lavoro: verrà riscritto separatamente. Riusare però le utility esistenti (`base/brep_utils`, `base/geom_utils`, `base/xcaf`).
- Rispettare lo stile di Mayo: `namespace Mayo`, `OccHandle<>`, file `snake_case.h/.cpp`, testi UI tramite il sistema di traduzione di Mayo (`MAYO_DECLARE_TEXT_ID_FUNCTIONS`).

---

## 2. Architettura

```
src/holes/                      ← libreria pura OCCT, NESSUNA dipendenza Qt
    hole_types.h                  struct dati (Hole, HoleSegment, HoleType…)
    hole_tolerances.h             TUTTE le tolleranze in un unico posto
    hole_recognizer.h/.cpp        geometria → lista di Hole
    hole_classifier.h/.cpp        Hole → tipo (regole + tabella diametri + colori)
    hole_color_profiles.h/.cpp    lettura/scrittura profili colore (JSON)
    hole_config.h/.cpp            tabella diametri e regole (JSON)
    hole_export.h/.cpp            export CSV / JSON
src/app/
    widget_hole_analysis.h/.cpp/.ui   pannello con tabella e filtri
    hole_table_model.h/.cpp           QAbstractItemModel (vista piatta + raggruppata)
    hole_graphics_overlay.h/.cpp      oggetti AIS colorati + trasparenza del pezzo
src/cli/ (o nuovo target)
    mayo-holes                     eseguibile da riga di comando: file.step → JSON
tests/
    test_holes.cpp                 test automatici su file campione
    inputs/holes/*.step + *.expected.json
```

Il riconoscimento deve poter girare **senza interfaccia** (CLI + test). La UI è solo un consumatore dei risultati.

Integrazione UI: nuovo pulsante "Analisi fori" nella barra di `WidgetGuiDocument`, sullo stesso schema di `m_btnMeasure` / `toggleWidgetMeasure()` e dentro `exclusiveButtonCheck()`. Il pannello deve contenere una tabella larga, quindi non usare il piccolo overlay dei pannelli esistenti. Usa un pannello laterale/inferiore ridimensionabile (QDockWidget o splitter): Claude Code sceglie la soluzione più pulita dopo aver letto `mainwindow.cpp` e `widget_main_control.cpp`, e motiva la scelta.

---

## 3. Modello dati

```cpp
enum class HoleEndKind { Open, FlatBottom, DrillTip, Other };   // come termina ciascun lato
enum class ClassSource { None, Geometry, DiameterTable, ColorProfile, Manual };

struct HoleSegment {              // un tratto coassiale a Ø costante (o conico)
    enum class Kind { Cylinder, Cone, Plane, Torus } kind;
    double diameter;              // per i coni: Ø maggiore e minore
    double diameterMin;
    double axialStart, axialEnd;  // parametri lungo l'asse del foro (mm)
    double angularCoverageDeg;    // 360 = cilindro completo
    std::vector<TopoDS_Face> faces;
};

struct Hole {
    int id;
    gp_Ax1 axis;                  // in coordinate GLOBALI (location dell'istanza applicata)
    gp_Pnt entryPoint;            // centro dell'ingresso principale, globale
    std::vector<HoleSegment> segments;   // ordinati dall'ingresso verso il fondo
    double mainDiameter;          // Ø del tratto "funzionale" (vedi §4.6)
    double lengthMax, lengthMin;  // lunghezza lungo l'asse; min≠max se l'ingresso è su superficie inclinata/free-form
    HoleEndKind endA, endB;
    bool through;
    double tipAngleDeg;           // se DrillTip (es. 118, 120, 140)
    std::optional<double> counterboreDiameter, counterboreDepth;
    std::optional<double> countersinkDiameter, countersinkAngleDeg;
    bool partial;                 // copertura angolare < 360°
    std::optional<Quantity_Color> color;      // colore facce dal file (se presente)
    std::vector<int> intersectsWith;          // id dei fori che lo intersecano (circuiti acqua)

    // classificazione
    std::string typeId;           // es. "threaded", "water", "through", "blind", "counterbore", "dowel", "unclassified"
    std::string designation;      // es. "M8", "G 1/8", "Ø6 acqua", "TCEI M10"
    ClassSource source;
    bool manualOverride;
};
```

---

## 4. Algoritmo di riconoscimento (`hole_recognizer`)

### 4.1 Input
- Il solido selezionato: un `TopoDS_Solid` scelto in 3D (selection mode `TopAbs_SOLID` sull'oggetto `XCAFPrs_AISObject`) oppure un nodo nell'albero del modello.
- **Lavorare sulla shape prototipo senza location** e applicare la location globale dell'istanza solo ai risultati (asse, punti). Così i colori XCAF (§5.4) combaciano e i componenti ripetuti non si ricalcolano.
- Se il nodo selezionato contiene più solidi, analizzarli tutti e indicare nella tabella a quale solido/componente appartiene ogni foro.

### 4.2 Classificazione delle facce (parallelizzabile con `OSD_Parallel::For`)
Per ogni faccia, `BRepAdaptor_Surface::GetType()`:
- `GeomAbs_Cylinder` → candidato tratto cilindrico
- `GeomAbs_Cone` → candidato punta / svasatura
- `GeomAbs_Plane` → candidato fondo piano / gradino di lamatura (vedi 4.4)
- `GeomAbs_Torus` → candidato raccordo coassiale (es. raggio sul fondo di una lamatura)
- `GeomAbs_BSplineSurface` / `SurfaceOfRevolution` → **fase 2**: provare a convertirle in cilindro/cono con `GeomConvert_SurfToAnaSurf` (alcuni CAD esportano cilindri come B-spline). Per ora ignorarle, ma contarle e segnalarle nel log.

### 4.3 Test di concavità (foro vs perno)
Per una faccia cilindrica:
1. Prendere un punto P a metà del dominio UV (`BRepTools::UVBounds`).
2. Calcolare la normale **tenendo conto dell'orientamento della faccia**: `BRepGProp_Face::Normal`, oppure `BRepLProp_SLProps` invertendo se `face.Orientation() == TopAbs_REVERSED`.
3. R = vettore dall'asse a P (componente radiale).
4. **Foro (concavo) se N·R < 0**: la normale uscente dal materiale punta verso l'asse.

Stesso principio per i coni e i tori. I cilindri convessi (perni, colonne, raccordi convessi) si scartano.

### 4.4 Raggruppamento per asse
1. **Clustering delle linee d'asse:** due facce stanno sullo stesso asse se le direzioni sono parallele entro `angTol` (±, il verso non conta) e la distanza di un punto dell'asse dall'altro asse è < `linTol`. Per evitare O(n²): hash su direzione quantizzata + punto dell'asse più vicino all'origine, poi verifica fine.
2. **Facce piane:** una faccia piana entra nel gruppo di un asse solo se è perpendicolare all'asse **e tutti i suoi spigoli sono cerchi centrati sull'asse** (fondo piano = disco, gradino di lamatura = corona circolare). Così si esclude la faccia superiore del blocco, che ha anche altri spigoli.
3. **Componenti connesse:** dentro ogni cluster d'asse, unire le facce adiacenti (mappa spigolo→facce con `TopExp::MapShapesAndAncestors(solid, TopAbs_EDGE, TopAbs_FACE, …)`). Ogni componente connessa = **un foro**. Due fori coassiali separati da materiale restano così due fori distinti.
4. **Semi-facce:** due facce dello stesso cilindro infinito (stesso asse, stesso raggio) con intervalli assiali sovrapposti si uniscono nello stesso `HoleSegment` e sommano la copertura angolare. Questo copre il caso di molti CAD che spezzano il cilindro in due metà.

### 4.5 Estensione assiale e lunghezza
- Per i cilindri, il parametro V di `Geom_CylindricalSurface` corrisponde alla distanza lungo l'asse. Si convertono i range V in un parametro assiale comune proiettando l'origine di ciascun cilindro sull'asse del foro.
- **Fallback obbligatorio:** se l'ingresso è su una superficie inclinata o free-form, lo spigolo d'ingresso non è un cerchio. In quel caso si discretizzano gli spigoli di bordo (`GCPnts_QuasiUniformDeflection`), si proiettano sull'asse e si prende min/max. Il risultato va in `lengthMax` e `lengthMin`: lunghezza sulla generatrice più lunga e su quella più corta. Negli stampi è il caso normale, non un'eccezione.

### 4.6 Ingresso, fondo, tipo di estremità
Per ciascuna delle due estremità del foro:
- **Open:** gli spigoli d'estremità confinano con facce che non fanno parte del foro.
- **DrillTip:** l'estremità è chiusa da un cono concavo coassiale. Angolo di punta = 2 × `gp_Cone::SemiAngle()` (es. 118°).
- **FlatBottom:** l'estremità è chiusa da un disco piano coassiale.
- **Passante:** entrambe le estremità sono Open.
- **Ingresso:** per un cieco è l'estremità Open. Per un passante è l'estremità del Ø maggiore se c'è lamatura o svasatura, altrimenti un estremo convenzionale, segnalato come "passante — direzione ambigua".
- **Lamatura:** tratto cilindrico di Ø maggiore all'ingresso seguito da corona piana (o toro), poi il foro. Si registrano Ø e profondità della lamatura.
- **Svasatura:** cono all'ingresso con Ø che decresce verso l'interno.
- **`mainDiameter`:** Ø del tratto più lungo escluse lamatura e svasatura. Su un foro a gradini con più tratti funzionali si usa il tratto più profondo e si mostrano tutti i tratti in un sotto-livello della tabella.

### 4.7 Casi da gestire esplicitamente
- **Circuiti acqua che si incrociano:** il foro attraversato viene spezzato in più facce non adiacenti. Regola: due tratti dello stesso cilindro infinito si uniscono nello stesso foro se l'intervallo tra loro è interamente occupato dal vuoto di un altro foro riconosciuto. In pratica, si uniscono se entrambi i tratti sono adiacenti alle facce dello stesso foro che li interseca e il gap ≤ Ø di quel foro + `linTol`. Si registra `intersectsWith` in entrambi i sensi.
- **Fori parziali** (mezzi fori su uno spigolo, asole, fori tagliati da una tasca): copertura angolare < `partialCoverageDeg` (default 300°) → `partial = true`, mostrati con un'icona dedicata.
- **Raccordi concavi** (non sono fori): copertura < `filletCoverageDeg` (default 180°) **e** tangenza con almeno una faccia adiacente → scartati. La tangenza si verifica con `BRep_Tool::Continuity` sullo spigolo; se l'informazione manca nello STEP, si confrontano le normali delle due facce nel punto medio dello spigolo.
- **Ø fuori range:** scartare i Ø < `minDiameter` (default 0,5 mm) e > `maxDiameter` (default 60 mm, configurabile). Così si escludono le cave grandi cilindriche.

### 4.8 Prestazioni
- Obiettivo: < 2 s per un solido con 20.000 facce sulla workstation dell'officina.
- Analisi in un thread separato con barra di avanzamento (riusare il sistema task di Mayo, vedi `dialog_task_manager`), annullabile.
- Niente tessellazione nel riconoscimento: lavora solo su B-rep esatto.

### 4.9 Tolleranze (`hole_tolerances.h`, tutte configurabili)
| Nome | Default | Uso |
|---|---|---|
| `linTol` | 0.001 mm | coassialità, coincidenze |
| `angTol` | 0.01° | parallelismo assi |
| `diamTol` | 0.002 mm | stesso Ø tra semi-facce |
| `partialCoverageDeg` | 300° | soglia "parziale" |
| `filletCoverageDeg` | 180° | soglia raccordo |
| `minDiameter` / `maxDiameter` | 0.5 / 60 mm | filtro |

---

## 5. Classificazione (`hole_classifier`)

### 5.1 Priorità — regola fissa
**Manuale > Profilo colore > Tabella diametri/regole geometriche > Non classificato.**
Ogni foro porta con sé la fonte (`ClassSource`), che è visibile in tabella. Un override manuale non viene mai sovrascritto da un ricalcolo.

### 5.2 Tabella diametri (JSON, modificabile dall'utente)
Esempio di default **da verificare e completare con la tabella dell'officina**:
```json
{
  "tolerance": 0.05,
  "entries": [
    { "diameter": 2.5,   "type": "threaded", "designation": "M3" },
    { "diameter": 3.3,   "type": "threaded", "designation": "M4" },
    { "diameter": 4.2,   "type": "threaded", "designation": "M5" },
    { "diameter": 5.0,   "type": "threaded", "designation": "M6" },
    { "diameter": 6.8,   "type": "threaded", "designation": "M8" },
    { "diameter": 8.5,   "type": "threaded", "designation": "M10" },
    { "diameter": 10.2,  "type": "threaded", "designation": "M12" },
    { "diameter": 12.0,  "type": "threaded", "designation": "M14" },
    { "diameter": 14.0,  "type": "threaded", "designation": "M16" },
    { "diameter": 8.8,   "type": "threaded", "designation": "G 1/8" },
    { "diameter": 11.8,  "type": "threaded", "designation": "G 1/4" },
    { "diameter": 15.25, "type": "threaded", "designation": "G 3/8" }
  ]
}
```
Un Ø può corrispondere a più voci (es. 12 = preforo M14 **o** foro acqua Ø12). In quel caso le regole del §5.3 decidono. Se restano ambigui, il foro va in "Non classificato" con i candidati elencati nel tooltip.

### 5.3 Regole geometriche (JSON, valutate in ordine, la prima che vince)
Esempi iniziali, **tutti da validare con Paolo**:
- **Lamatura TCEI:** lamato con Ø lamatura e Ø foro compatibili con la tabella viti TCEI (es. Ø9/Ø14 → TCEI M8) → `counterbore`, designazione "TCEI M8".
- **Acqua:** L/D > `waterMinLD` (es. 8) e Ø nella lista acqua (es. 6, 8, 10, 12, 14), oppure foro che interseca altri fori dello stesso tipo, oppure ingresso con preforo gas (G 1/8, G 1/4…) seguito da un tratto di Ø minore → `water`.
- **Spina / alesato:** Ø intero con tratto passante o cieco corto, senza lamatura (regola debole, solo come candidato).
- **Passante liscio / cieco liscio:** fallback se nessuna regola precedente vale.

### 5.4 Colori
- **Lettura:** `XCAFPrs::CollectStyleSettings(label, TopLoc_Location(), styles)` sulla label del prototipo restituisce lo stile per sotto-shape. Priorità del colore: faccia > solido > parte. Colore del foro = colore prevalente delle sue facce. Se le facce dello stesso foro hanno colori diversi, si usa il colore della faccia più estesa e si segnala il caso.
- **Raggruppamento anche senza significato:** i fori con lo stesso colore (entro `colorTol`, distanza in spazio RGB o ΔE) formano un gruppo visibile in tabella, es. "RGB 255/0/0 — 24 fori — Ø6,8". Il gruppo esiste anche quando nessun profilo è attivo.
- **Profili colore per cliente** (JSON in `%APPDATA%/Mayo/hole_profiles/`):
  ```json
  { "name": "Cliente Rossi", "colorTol": 8,
    "entries": [ { "rgb": [255,0,0], "type": "threaded" },
                 { "rgb": [0,0,255], "type": "water" } ] }
  ```
- **Apprendimento:** click destro su un gruppo colore → "Assegna tipo…" → il colore viene salvato nel profilo attivo (o in uno nuovo).
- **Proposta automatica:** all'apertura di un file colorato, per ogni profilo si calcola il punteggio = (fori colorati il cui colore è nel profilo) / (fori colorati totali). Se il migliore supera 0,6 viene proposto in una barra non invasiva: "Sembra il profilo *Cliente Rossi* — applica / scegli altro / ignora". **Mai applicare in automatico senza conferma.**

---

## 6. Interfaccia

### 6.1 Flusso
1. Pulsante "Analisi fori" → si attiva la selezione per solido.
2. L'utente clicca un solido in 3D o seleziona un nodo nell'albero → pulsante "Analizza".
3. Analisi in background → si riempie la tabella → la vista 3D passa in "modalità fori".
4. Chiudendo il pannello si ripristina tutto com'era: trasparenze, visibilità, selezione.

### 6.2 Tabella
- **Vista raggruppata (default):** Tipo → Designazione/Ø → singoli fori, con quantità per gruppo.
- **Vista piatta:** un foro per riga, ordinabile su qualsiasi colonna (`QSortFilterProxyModel`).
- **Colonne:** ID · Tipo · Designazione · Ø · L max · L min · Cieco/Passante · Fondo (punta 118° / piano / —) · Lamatura (Ø × prof.) · Svasatura · X · Y · Z ingresso · Direzione (i, j, k) · Colore · Fonte classificazione · Intersezioni · Componente.
- **Filtri:** tipo, intervallo di Ø, direzione, solo non classificati, solo parziali.
- **Modifica:** il tipo è modificabile da combo nella cella o in multi-selezione. La modifica imposta `Manual` e diventa grassetto o con un'icona.
- **Riferimento:** combo "Assoluto / riferimento custom" per X, Y, Z e direzione. Il riferimento custom è lo stesso che si usa per le sezioni (oggi i clip plane di Mayo, `widget_clip_planes`). Prevedere un'interfaccia comune `IReferenceFrameProvider` per non duplicarlo.
- **Export:** CSV con separatore `;` e virgola decimale (Excel italiano), con opzione punto decimale. Più export JSON completo.
- **Formato numeri:** Ø a 3 decimali, lunghezze a 2.

### 6.3 Vista 3D
- Il pezzo (l'oggetto `XCAFPrs_AISObject` del documento) va in trasparenza (`AIS_InteractiveContext::SetTransparency`, default 0,8, regolabile con slider) e senza selezione attiva.
- **Un oggetto `AIS_Shape` per gruppo** (compound delle facce dei fori del gruppo, con location globale), colorato col colore del gruppo. Colori di default per tipo, modificabili dalla legenda.
- Polygon offset sugli overlay per evitare lo z-fighting con il pezzo trasparente.
- Gli overlay devono essere **sensibili ai piani di sezione** (`GraphicsScene::setObjectClipPlaneSensitive`), così sezioni e analisi fori lavorano insieme.
- **Selezione sincronizzata:** click su riga ↔ evidenziazione in 3D. Doppio click su riga → zoom sul foro. Click sul foro in 3D → seleziona la riga.
- **Tooltip in hover sul foro:** tipo, designazione, Ø, L, fonte.
- Opzioni: "Nascondi altri componenti", "Mostra asse dei fori" (linee tratteggiate), "Mostra solo gruppo selezionato".

### 6.4 Direzioni / piazzamenti (utile per il 3+2)
- Raggruppare i fori per direzione d'asse (entro `angTol`, considerando ±).
- Mostrare l'elenco delle direzioni con il numero di fori e due angoli generici: inclinazione rispetto a Z e azimut nel piano XY, nel riferimento scelto.
- Riga riassuntiva: "N direzioni diverse". La conversione negli angoli della tavola Detron resta **fuori** da questa versione: serve la cinematica esatta della macchina.

---

## 7. Test e validazione

- Paolo fornisce un set di STEP campione in `tests/inputs/holes/`:
  1. blocco semplice con fori ciechi/passanti/filettati/lamati
  2. piastra figura reale di uno stampo
  3. piastra con circuito acqua incrociato
  4. stesso pezzo colorato e non colorato
  5. pezzo con fori su superfici free-form/inclinate
  6. un assieme grande (test prestazioni)
- Per ogni file, un `.expected.json` con numero di fori per tipo/Ø. **Validazione incrociata con l'analisi fori di WorkNC sugli stessi file:** le differenze vanno spiegate una per una.
- `mayo-holes file.step --json out.json` per i test automatici e per il confronto rapido.
- I test girano con il framework di test già presente in Mayo (`tests/`, Qt Test).

---

## 8. Tappe di sviluppo (ognuna chiusa solo se compila, i test passano e Paolo l'ha provata)

| # | Tappa | Criterio di accettazione |
|---|---|---|
| M0 | Fork, build Windows (vcpkg OCCT + Qt6), esecuzione | Mayo compilato apre uno STEP grande dell'officina |
| M1 | `src/holes` riconoscimento §4.1–4.6 + CLI `mayo-holes` | Su file 1 e 2: numero di fori, Ø e L corretti vs WorkNC |
| M2 | Casi §4.7 (acqua, parziali, raccordi, free-form) | File 3 e 5 corretti; nessun raccordo scambiato per foro |
| M3 | Classificazione §5.1–5.3 con JSON configurabili | Filettati/lamati/acqua riconosciuti sul file 2 senza colori |
| M4 | UI: tabella + overlay 3D + selezione sincronizzata | Flusso §6.1 completo, ripristino corretto della vista |
| M5 | Colori §5.4: gruppi, profili, apprendimento, proposta | File 4: con colori il profilo viene proposto e applicato |
| M6 | Export, riferimento custom, direzioni §6.4, prestazioni | Assieme grande analizzato entro l'obiettivo §4.8 |

---

## 9. Punti aperti (decide Paolo)

1. Elenco definitivo dei tipi di foro e colori di default per tipo.
2. Tabella preforo/filetti e viti TCEI dell'officina (sostituisce i default del §5.2).
3. Regole "acqua" e "spina": soglie reali usate in officina.
4. La profondità del filetto, visto che di solito non è nello STEP: campo manuale in tabella o ignorata?
5. Ø massimo da considerare foro (default 60 mm).
6. Formato CSV: quali colonne servono davvero per il preventivo e per il programma.

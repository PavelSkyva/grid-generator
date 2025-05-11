# DRN Generator

`drn-generator` je nástroj spouštěný z příkazové řádky pro generování `.drn` modelů určených pro prostředí typu POMDP. Dokáže buď náhodně generovat modely, nebo převést zadaný obrázek ve formátu gridu do formátu `.drn`.


## 🔧 Kompilace

K překladu nástroje spusťte:

```bash
make
```

### Požadavky

- Kompilátor s podporou C++17 (např. `g++`)
- GNU Make

Výsledkem je spustitelný soubor `drn-generator`.

## ▶️ Spouštění skriptu

```
./drn-generator
    -matrix <název souboru s obrázkem matice>, pokud není zadán, bude grid vygenerován náhodně!
    -rows <počet_řádků>
    -cols <počet_sloupců>
    --impass <pravděpodobnost, že agent neprojde>
    --slippery <pravděpodobnost, že agent uklouzne kolmo k dané akci>
    -samples <počet vygenerovaných modelů při náhodném generování, při předaném gridu je samozřejmě vygenerován jen jeden soubor>
```

Na pořadí parametrů nezáleží.

## 📄 Parametry

| Parametr             | Popis                                                                                                           |
|----------------------|-----------------------------------------------------------------------------------------------------------------|
| `-matrix <soubor>`   | Název souboru s obrázkem gridu prostředí. Pokud není zadán, bude mřížka vygenerována náhodně.                 |
| `-rows <int>`        | Počet řádků prostředí (povinný argument).                                    |
| `-cols <int>`        | Počet sloupců prostředí (povinný argument).                                  |
| `--impass <float>`   | Pravděpodobnost, že agent neprojde (tj. akce selže úplně).                                                      |
| `--slippery <float>` | Pravděpodobnost, že agent uklouze kolmo k zamýšlené akci.                                                        |
| `-samples <int>`     | Počet modelů, které budou vygenerovány při náhodném generování. Pokud je zadán `-matrix`, vygeneruje se jen jeden model. |

## 📤 Výstup

Vygenerované `.drn` soubory se uloží do aktuálního adresáře nebo do specifikované složky podle konfigurace projektu.

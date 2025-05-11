# DRN Generator

`drn-generator` je nástroj spouštěný z příkazové řádky pro generování `.drn` modelů určených pro prostředí typu POMDP. Dokáže buď náhodně generovat modely založené na gridu, nebo převést zadaný obrázek matice do formátu `.drn`.


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
    --impass <pravděpodobnost, že agent uklouzne>
    --slippery <pravděpodobnost, že agent půjde kolmo k dané akci>
    -samples <počet vygenerovaných modelů při náhodném generování, při předaném gridu je samozřejmě vygenerován jen jeden soubor>
```

Na pořadí parametrů nezáleží.

## 📄 Parametry

| Parametr             | Popis                                                                                                           |
|----------------------|-----------------------------------------------------------------------------------------------------------------|
| `-matrix <soubor>`   | Název souboru s obrázkem matice prostředí. Pokud není zadán, bude mřížka vygenerována náhodně.                 |
| `-rows <int>`        | Počet řádků generovaného prostředí (používá se jen při náhodném generování).                                    |
| `-cols <int>`        | Počet sloupců generovaného prostředí (používá se jen při náhodném generování).                                  |
| `--impass <float>`   | Pravděpodobnost, že agent uklouzne (tj. akce selže úplně).                                                      |
| `--slippery <float>` | Pravděpodobnost, že agent vykoná akci kolmo k zamýšlené.                                                        |
| `-samples <int>`     | Počet modelů, které budou vygenerovány při náhodném generování. Pokud je zadán `-matrix`, vygeneruje se jen jeden model. |

## 📤 Výstup

Vygenerované `.drn` soubory se uloží do aktuálního adresáře nebo do specifikované složky podle konfigurace projektu.

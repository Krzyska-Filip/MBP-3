# Miniprojekt 3 — Algorytmy wzajemnego wykluczania

## Struktura programów

### `main`
Każdy plik ma identyczną strukturę funkcji `main`:
1. Uruchomienie pomiaru czasu
2. Stworzenie wątków
3. Zatrzymanie pomiaru czasu
4. Wyświetlenie wyników
5. Zapis do pliku

### Worker
Każdy worker działa tak samo:
1. Uruchamia pętlę
2. Zakłada zamek wybranym algorytmem (Peterson / TAS / TTAS)
3. Inkrementuje licznik

## Pliki

### Algorytm Petersona

| Plik | Opis | Slajd | Priorytet |
|------|------|-------|-----------|
| [p_br.c](p_br.c) | Peterson — wersja zepsuta | 142 | **ważny** |
| [p_ok.c](p_ok.c) | Peterson — wersja naprawiona | 143 | **ważny** |
| [p_fair.c](p_fair.c) | Peterson naprawiony + pomiar fairness (ile dostępu dostaje każdy wątek) | 143 | średnio ważny |
| [p_weak.c](p_weak.c) | To samo co `p_ok.c`, ale z innymi zamkami (bliżej tego co jest na slajdach) | — | mało ważny |
| [p_ttas.c](p_ttas.c) | Peterson z TTAS — w tym kontekście nic nie daje | — | mało ważny |

### TAS / TTAS

| Plik | Opis | Slajd |
|------|------|-------|
| [tas.c](tas.c) | TAS — implementacja 1:1 z prezentacji | 146 |
| [ttas.c](ttas.c) | TTAS — implementacja 1:1 z prezentacji | 146 |

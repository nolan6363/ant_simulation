# Ant simulation

Simulation d'une colonie de fourmis en C, parallélisée avec OpenMP et pthreads.
Projet de Parallel Programming (ING5), par Nolan BAYON et Thomas MARGOTTEAU.

## Contenu

- `sequential/` : version séquentielle de référence
- `parallel/` : version parallèle (même code, compilé avec `-fopenmp`)
- `report/` : rapport et slides (sources markdown et PDF)

## Dépendances

- gcc, make, pkg-config
- SDL3 et SDL3_ttf (affichage et export des images)
- la police DejaVu Sans Mono (`fonts-dejavu-core`)

Sur Raspberry Pi OS, si SDL3 n'est pas disponible avec `apt`, il faut le compiler depuis les sources avec CMake.

## Compilation

```
make -C sequential
make -C parallel
```

## Utilisation

```
./bin/main                              # mode fenêtré
./bin/main <steps> [saveEvery] [dumpFile]
```

- `steps` : nombre de steps, 0 pour aller jusqu'à ce qu'il n'y ait plus de nourriture
- `saveEvery` : sauvegarde une image BMP dans `frames/` tous les N steps (0 = aucune)
- `dumpFile` : écrit l'état final dans un fichier (utilisé par les tests)

Le nombre de threads se choisit avec `OMP_NUM_THREADS` :

```
cd parallel
OMP_NUM_THREADS=4 ./bin/main 0
```

En mode fenêtré : espace pour mettre en pause, 1/2/3 pour afficher ou cacher les fourmis et les phéromones, clic gauche pour dessiner un mur, clic droit pour l'effacer, échap pour quitter.

## Tests

```
make -C parallel test
```

Lance la simulation complète en séquentiel puis en parallèle avec 1, 2, 3 et 4 threads, et vérifie que l'état final est identique.

## Rapport et slides

```
cd report
pandoc rapport.md -o rapport.pdf
pandoc -t beamer presentation.md -o presentation.pdf
```

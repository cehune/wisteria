# scripts

This folder is for python scripts that are more intuitive to run and use than a CPP version would be. 
All output results are saved in outputs/{script_name}

## pfm_to_png.py
Saving an image should output the pfm to the wisteria/outputs folder at the repository base. 
In order to convert that to a png that we can visually see, use the pfm_to_png script. 
```
python3 pfm_to_png "filepath"
etc
python3 .../wisteria/outputs/pfm/wisteria_76spp.pfm
```
This would save a png image, with the same name and the png extension at `outputs/pfm_to_png/wisteria_76spp.png`
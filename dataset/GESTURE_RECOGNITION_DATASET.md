# Hand Gesture Recognition Dataset



## Dataset Structure
- ```train.pkl```: tranining dataset (12,800 samples)
- ```cal.pkl```: calibration dataset (1,600 samples)
- ```test.pkl```: test dataset (1,600 samples)
- ```dataset.ipynb```: data preprocessing and generation notebook

Each pickle file contains a tuple of $(X, y)$ where:
- $X$: image data array of shape (n_samples, 96, 96), normalized to $[0, 1]$
- $y$: labels array of sahpe (n_samples)


## Gesture classes
The dataset inclues $8$ gesture types:
- 0: palm
- 1: I
- 2: fist
- 3: thumb
- 4: index
- 5: ok
- 6: c
- 7: down


## Data Charecteristics
- image size: $96 \times 96$ pixels
- color format: grayscale
- value range: 0-1 (float32)
- total samples: 16,000
- split ratio: 80% train, 10% calibration, 10% test


## Preprocessing steps
1. Convert to grayscale
2. Resize to $96 \times 96$
3. Normalize pixel values to $[0, 1]$
4. Split into train/cal/test sets
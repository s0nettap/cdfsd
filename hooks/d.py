import os
import shutil

# Текущая директория (где находится скрипт)
current_dir = os.path.dirname(os.path.abspath(__file__))

# Проход по всем файлам в директории
for filename in os.listdir(current_dir):
    original_path = os.path.join(current_dir, filename)

    # Пропустить папки и уже скопированные файлы
    if os.path.isfile(original_path) and not filename.endswith('.txt'):
        new_filename = filename + '.txt'
        new_path = os.path.join(current_dir, new_filename)

        try:
            shutil.copyfile(original_path, new_path)
            print(f'Скопирован: {filename} → {new_filename}')
        except Exception as e:
            print(f'Ошибка при копировании {filename}: {e}')

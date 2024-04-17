from pathlib import Path
from subprocess import CalledProcessError, run
from concurrent.futures import ThreadPoolExecutor

def is_video(file_path):
    video_extensions = {'.mp4', '.mkv', '.mov', '.avi'}
    return file_path.suffix.lower() in video_extensions

def encode_video(source_path, destination_directory):
    output_path = destination_directory / (source_path.stem + '.mp4')
    if output_path.exists():
        print(f"Skip existing file: {output_path}")
        return
    command = [
        'ffmpeg', '-i', str(source_path),
        '-vf', 'scale=320:240',  # Optimized resolution for 2.8-inch screen
        '-c:v', 'libx264', '-profile:v', 'baseline', '-level', '3.0',
        '-preset', 'medium',  # Balance between encoding speed and file size
        '-crf', '26',  # Moderately high compression to reduce file size
        '-pix_fmt', 'yuv420p', str(output_path)
    ]
    try:
        run(command, check=True)
        print(f'Encoded successfully: {output_path}')
    except CalledProcessError as e:
        print(f'Failed to encode {source_path}: {e}')

def find_videos(directory):
    return [path for path in directory.rglob('*') if path.is_file() and is_video(path)]

def main(directory_path):
    directory = Path(directory_path)
    destination_directory = directory / 'encoded'
    destination_directory.mkdir(exist_ok=True)
    
    video_files = find_videos(directory)
    with ThreadPoolExecutor() as executor:
        for video_file in video_files:
            executor.submit(encode_video, video_file, destination_directory)

if __name__ == '__main__':
    main(Path(__file__).parent.resolve())

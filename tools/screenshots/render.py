"""Regenerate the checked-in gallery with an already-built native renderer."""
from pathlib import Path
import argparse, hashlib, json, subprocess, tempfile
from PIL import Image

root = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--build-dir', type=Path, default=root/'build-screenshots')
parser.add_argument('--output-dir', type=Path, default=root/'docs/screenshots')
args = parser.parse_args()
manifest = json.loads((root/'docs/screenshots/manifest.json').read_text())
args.output_dir.mkdir(parents=True, exist_ok=True)
with tempfile.TemporaryDirectory(prefix='jet-screenshots-') as temporary:
    for entry in manifest['captures']:
        name = entry['example']
        candidates = [directory/(name+suffix) for directory in
                      (args.build_dir, args.build_dir/'Release') for suffix in ('', '.exe')]
        executable = next((p for p in candidates if p.is_file()), None)
        if not executable:
            raise FileNotFoundError(f'Build the screenshot target {name} first.')
        ppm = Path(temporary)/'frame.ppm'
        subprocess.run([str(executable.resolve()), str(entry['seconds']), str(ppm)], check=True)
        output = args.output_dir/entry['file']
        with Image.open(ppm) as picture:
            if picture.size != (480, 320):
                raise ValueError(f'Unexpected capture dimensions: {picture.size}')
            picture.save(output)
        entry['sha256'] = hashlib.sha256(output.read_bytes()).hexdigest()
        config = root/name/'main/firmware/JetConfig.hpp'
        # Hash the repository's LF form, independent of checkout line endings.
        entry['config_sha256'] = hashlib.sha256(config.read_bytes().replace(b'\r\n', b'\n')).hexdigest()
manifest['jet_commit'] = subprocess.check_output(
    ['git', '-C', str(root/'components/Jet'), 'rev-parse', 'HEAD'], text=True).strip()
(args.output_dir/'manifest.json').write_text(json.dumps(manifest, indent=2)+'\n')

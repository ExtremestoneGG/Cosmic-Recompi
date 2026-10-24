"""Relocate the integration includes in an extracted corresponding-source archive."""
import argparse
from pathlib import Path
import re


def relocate(sdk: Path, consumer: Path) -> int:
    sdk = sdk.resolve(strict=True)
    consumer = consumer.resolve(strict=True)
    integration = consumer / 'native-rex/src'
    if not (sdk/'src/graphics/d3d12/render_target_cache.cpp').is_file():
        raise ValueError('Expected the extracted rexglue-sdk directory')
    if not integration.is_dir():
        raise ValueError('Expected consumer/native-rex/src')
    # Match the old absolute project prefix without depending on a drive or
    # user name. Only named headers present in the consumer may be relocated.
    pattern = re.compile(r'(#include\s+")(?:@COSMIC_CONSUMER@/|[A-Za-z]:/[^"\r\n]*/native-rex/src/)([^"\r\n]+)(")')
    pending = []
    for base in (sdk/'src', sdk/'include'):
        for path in base.rglob('*'):
            if not path.is_file() or path.suffix not in ('.cpp', '.h'):
                continue
            original = path.read_bytes()
            text = original.decode('utf-8')
            def replace(match):
                target = (integration/match.group(2)).resolve(strict=True)
                target.relative_to(integration.resolve())
                return match.group(1) + target.as_posix() + match.group(3)
            adjusted = pattern.sub(replace, text).encode('utf-8')
            if adjusted != original:
                pending.append((path, adjusted))
    # Validate all headers first, before modifying any SDK file.
    for path, adjusted in pending:
        path.write_bytes(adjusted)
    return len(pending)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk', type=Path, required=True)
    parser.add_argument('--consumer', type=Path, required=True)
    args = parser.parse_args()
    print(f'Relocated {relocate(args.sdk, args.consumer)} SDK source files.')

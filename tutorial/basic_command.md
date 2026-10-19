# Basic Commands
## WSL

```powershell
# wsl 시스템 설치
wsl --install --no-distribution
# ubuntu-24.04 배포판 설치
wsl --install -d ubuntu-24.04
# wsl 배포판 확인
wsl --lisst
# wsl 상태 확인
wsl --status
```

## linux 
```bash
ls -al
mkdir <dir>
cd <dir>
# 권한 변경
chmod <권한번호> <dir>
```

## docker
```bash
# 도커 이미지 확인
docker images
# 도커 이미지 다운로드
docker pull espressif/idf:v6.0.2
# 도커 실행 명령어 확인
docker run --help
# 도커 실행 옵션
docker run -it --rm -u 1000:1000 -w /home/ubuntu/esp-ief -v $(pwd):/home/ubuntu/esp-idf -v /dev:/dev --privileged espressif/idf:v6.0.2 bash 
```

## esp idf 명령어
```bash
# 도움말
idf.py --help
# 프로젝트 생성 
idf.py create-project <project_name>
# 타켓 설정
idf.py set-target esp32s3
# 메뉴설정
idf.py menuconfig
# 빌드
idf.py build
# 결과 에뮬레이션
idf.py qemu
# falsh
idf.py flash
```


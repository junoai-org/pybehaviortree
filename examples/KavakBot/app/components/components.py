import os
from asyncio import Lock
from pathlib import Path
from typing import Dict, Any

from examples.KavakBot.app.components.cache.cache_interface import CacheInterface
from examples.KavakBot.app.components.cache.redis import AsyncRedis
from examples.KavakBot.app.components.configuration.configuration import Configuration
from examples.KavakBot.app.components.configuration.configuration_interface import ConfigurationInterface


class ComponentsMeta(type):
    _instances: Dict = {}
    _lock: Lock = Lock()

    def __call__(cls, *args, **kwargs):
        with cls._lock:
            if cls not in cls._instances:
                instance = super().__call__(*args, **kwargs)
                cls._instances[cls] = instance
        return cls._instances[cls]


class Components(metaclass=ComponentsMeta):

    def __init__(self, env: str, config_path: str) -> None:
        self.__env: str = env
        root_dir: str = str(Path(__file__).resolve().parents[2])
        self.__config_path: str = os.path.join(root_dir, config_path)
        self.__components: Dict[str, Any] = self.__bootstrap_components()

    def __bootstrap_components(self) -> Dict[str, Any]:
        if self.__env == 'development':
            return self.__get_dev_components()

        raise ValueError(f'Invalid environment: {self.__env}')

    def __get_dev_components(self) -> Dict[str, Any]:
        configuration: ConfigurationInterface = Configuration(self.__env, self.__config_path)
        redis: CacheInterface = AsyncRedis(
            configuration.get_configuration('REDIS_HOST', str),
        )

        return {
            'configuration': configuration,
            'cache': redis,
        }

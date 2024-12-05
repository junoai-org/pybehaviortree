import copy
import json
import os
from typing import Type, Any, Dict, Optional, Union
from collections.abc import Mapping

from examples.KavakBot.app.components.configuration.configuration_interface import ConfigurationInterface


class Configuration(ConfigurationInterface):
    def __init__(self, env: str, config_path: str) -> None:
        default_config_path = os.path.join(config_path, 'default.json')
        env_config_path = os.path.join(config_path, f'{env}.json')

        with open(default_config_path) as default_file:
            default_config: Dict[str, Any] = json.load(default_file)
        with open(env_config_path) as env_file:
            env_config: Dict[str, Any] = json.load(env_file)

        self.__data: Dict[str, Any] = self.__deep_merge(default_config, env_config)

    def __deep_merge(
            self,
            base: Dict[str, Any],
            update: Dict[str, Any]
    ) -> Dict[str, Any]:
        for key, value in update.items():
            if (
                    key in base
                    and isinstance(base[key], dict)
                    and isinstance(value, dict)
            ):
                base[key] = self.__deep_merge(base[key], value)
            else:
                base[key] = copy.deepcopy(value)
        return base

    def __get_configuration(
            self,
            config_name: str,
            default: Any,
            config_path: str
    ) -> Any:
        data: Dict[str, Any] = self.__data
        if config_path:
            for path in config_path.split('.'):
                if not isinstance(data.get(path), dict):
                    return default
                data = data[path]
        return copy.deepcopy(data.get(config_name, default))

    def get_configuration(
            self,
            config_name: str,
            config_type: Type[Any],
            default: Any = None,
            config_path: str = ''
    ) -> Any:
        value = self.__get_configuration(config_name, default, config_path)
        if value is None:
            return default
        if not isinstance(value, config_type):
            raise ValueError(
                f'El valor de configuración para "{config_name}" no es del tipo esperado {config_type.__name__}'
            )
        return value
